/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: camera_ctrl_play.c 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2017-4-9    
*                
* @brief:          
*                  
*                  
* @details:        
*                 
*    
*    
* @comment           
*******************************************************************************/
#include<stdio.h>
#include<pthread.h>
#include<es_video.h>
#include<es_display.h>

#define ES_DEBUG 1
#include<es_common.h>
#include<es_media_frame.h>
#include<es_frame_convert.h>

#define BRIGHTNESS_CTRL_ID 0x980900

typedef struct video_dsc{
	es_video_hld v_hld;
	struct es_video_attr v_attr;
	pthread_t backend_tid;
	pthread_mutex_t lock;
} video_dsc;

struct es_video_ctrl *ctrl_arr[32];
static void *vframe_update(void *arg);
static int set_camera_ctrl(struct es_video_ctrl *ctrl_item);

static struct video_dsc vdsc = {
	.v_hld = NULL,
	.lock  = PTHREAD_MUTEX_INITIALIZER,
};

int main(int argc, char *argv[])
{
	es_error_t ret = ES_SUCCESS;
	int err, i;
	
	ret = es_video_init();
	check_ret(ret, "es_video_init");

	ret = es_display_init();
	check_ret(ret, "es_display_init");

	ret = es_frame_convert_init();
	check_ret(ret, "es_frame_convert_init");

	pthread_mutex_lock(&vdsc.lock);
	ret = es_video_open("/dev/video15", ES_VIDEO_CLASS_CAMERA, &vdsc.v_hld);
	pthread_mutex_unlock(&vdsc.lock);
	check_ret(ret, "es_video_open");

	pthread_mutex_lock(&vdsc.lock);
	ret = es_video_get_attr(vdsc.v_hld, &vdsc.v_attr);
	pthread_mutex_unlock(&vdsc.lock);
	check_ret(ret, "es_video_get_attr");
	printf("[/dev/video15]: property is %d \n", vdsc.v_attr.property);
	printf("[/dev/video15]: pixel format is %d \n", vdsc.v_attr.pix_fomat);
	printf("[/dev/video15]: bytes per pixel is %d \n", vdsc.v_attr.bpp);
	printf("[/dev/video15]: resolution x : %d, y : %d\n", vdsc.v_attr.resolution.x, vdsc.v_attr.resolution.y);
	for(i = 0; i < 32; i++)
	{
		ctrl_arr[i] = NULL;
	}
	struct es_video_ctrl *cur_ctrl = NULL;
	struct es_video_ctrl *tmp_ctrl = NULL;
	i = 0;
	es_list_for_each_entry_safe(cur_ctrl, tmp_ctrl, vdsc.v_attr.p_ctrl_list_head, ctrl_entry)
	{
		if(NULL != cur_ctrl)
		{
			printf("\n");
			printf("[/dev/video15]: cur_ctrl->ctrl_id: 0x%x \n", cur_ctrl->ctrl_id);
			printf("[/dev/video15]: cur_ctrl->name: %s \n", cur_ctrl->name);
			printf("[/dev/video15]: cur_ctrl->minimum: %d \n", cur_ctrl->minimum);
			printf("[/dev/video15]: cur_ctrl->maximum: %d \n", cur_ctrl->maximum);
			printf("[/dev/video15]: cur_ctrl->step: %d \n", cur_ctrl->step);
			printf("[/dev/video15]: cur_ctrl->default_value: %d \n", cur_ctrl->default_val);
			printf("[/dev/video15]: cur_ctrl->status: 0x%x \n", cur_ctrl->status);
			printf("\n");
			ctrl_arr[i] = cur_ctrl;
			i++;
		}
	}
	pthread_mutex_lock(&vdsc.lock);
	ret = es_video_start(vdsc.v_hld);
	check_ret(ret, "es_video_start");
	err = pthread_create(&vdsc.backend_tid, NULL, vframe_update, NULL);
	pthread_mutex_unlock(&vdsc.lock);
	if(err)
	{	
		printf("pthread_create fail!!!\n");
		return -1;
	}
	
	while(1)
	{
		printf("--------ctrl list start-------------\n");
		printf("\n");
		for(i = 0; i < 32; i++)
		{
			if(NULL != ctrl_arr[i])
				printf("%d. %s\n", i, ctrl_arr[i]->name);
		}
		printf("--------ctrl list end-------------\n");
		printf("\n");
		unsigned long nr;
		printf("pls select num\n");
		scanf("%ul", &nr);
		nr %= 32;
		printf("nr: %ul\n", nr);
		if(NULL != ctrl_arr[nr])
			set_camera_ctrl(ctrl_arr[nr]);
	
		// in = getchar();
		//printf("input char: %c \n", in);
		// es_common_delay(100);
	}

	
	return 0;
}

static void *vframe_update(void *arg)
{
	es_error_t ret = ES_SUCCESS;
	es_disp_hld d_hld = NULL;
	struct es_display_attr d_attr;
	convert_hld c_hld = NULL;
	struct es_media_frame *vframe = NULL;



	ret = es_display_open("/dev/fb0", ES_DISPLAY_CLASS_FB, &d_hld);
	check_ret(ret, "es_display_open");

	ret = es_display_get_attr(d_hld, &d_attr);
	check_ret(ret, "es_display_get_attr");


	ret = es_frame_convert_open(ES_PIXEL_FRAME, &c_hld);
	check_ret(ret, "es_frame_convert_open ES_PIXEL_FRAME");
	while(1)
	{
		// es_common_delay(10000);
		vframe = es_media_frame_create();
		pthread_mutex_lock(&vdsc.lock);
		ret = es_video_sync_recv_frame(vdsc.v_hld, vframe);
		pthread_mutex_unlock(&vdsc.lock);
		
		check_ret(ret, "es_video_sync_recv_frame");
		if(vframe->attr.pix_frame.pix_fmt != d_attr.pix_fmt)
		{
			struct es_media_frame *dframe = NULL;
			dframe = es_convert_to_spec_frame_fmt(c_hld, vframe, d_attr.pix_fmt);
			if(NULL != dframe)
			{
				ret = es_display_sync_flush(d_hld, dframe);
				check_ret(ret, "es_display_sync_flush");
				es_media_frame_destroy(dframe);
				es_media_frame_destroy(vframe);
			}
			else
			{
				es_media_frame_destroy(vframe);
			}

		}
		else
		{
			vframe->attr.pix_frame.bpp = 32;
			vframe->attr.pix_frame.pix_fmt = ES_PIX_FMT_BGRA32;
			es_display_sync_flush(d_hld, vframe);
			es_media_frame_destroy(vframe);
		}
	}

}

static int set_camera_ctrl(struct es_video_ctrl *ctrl_item)
{
	es_error_t ret = ES_SUCCESS;
		
	if(NULL == ctrl_item)
		return -1;

	printf("\n");
	printf("[/dev/video15]: ctrl_item->name: %s \n", ctrl_item->name);
	printf("[/dev/video15]: ctrl_item->minimum: %d \n", ctrl_item->minimum);
	printf("[/dev/video15]: ctrl_item->maximum: %d \n", ctrl_item->maximum);
	printf("[/dev/video15]: ctrl_item->step: %d \n", ctrl_item->step);
	printf("\n");
	while(1)
	{
		char in = 'a';
		int val = 0;
		struct es_video_ctrl_cmd cmd;
		printf("----------set ctrl----------\n");
		printf("enter \'U\' to up \n");
		printf("enter \'D\' to down \n");
		printf("enter \'Q\' to quit \n");
		scanf("%c", &in);
		
		cmd.ctrl_id = ctrl_item->ctrl_id;
		pthread_mutex_lock(&vdsc.lock);
		ret = es_video_get_ctrl(vdsc.v_hld, &cmd);
		pthread_mutex_unlock(&vdsc.lock);
		if(ES_SUCCESS != ret)
		{
			printf("es_video_get_ctrl fail \n");
			return -1;;
		}
		printf("cmd.current_val: %d\n", cmd.current_val);
		switch(in)
		{
			case 'u':
			case 'U':
				printf("enter val to up \n");
				scanf("%d", &val);
				cmd.current_val += ctrl_item->step * val;
				pthread_mutex_lock(&vdsc.lock);
				ret = es_video_set_ctrl(vdsc.v_hld, &cmd);
				pthread_mutex_unlock(&vdsc.lock);
				if(ES_SUCCESS != ret)
				{
					printf("es_video_set_ctrl fail \n");
					return -1;;
				}
				break;
			case 'd':
			case 'D':
				printf("enter val to down \n");
				scanf("%d", &val);
				cmd.current_val -= ctrl_item->step * val;
				pthread_mutex_lock(&vdsc.lock);
				ret = es_video_set_ctrl(vdsc.v_hld, &cmd);
				pthread_mutex_unlock(&vdsc.lock);
				if(ES_SUCCESS != ret)
				{
					printf("es_video_set_ctrl fail \n");
					return -1;;
				}
				break;
			case 'q':
			case 'Q':
				return 0;
			default:
				printf("useless input!!! \n");
				break;
		}

	}
	return 0;
}