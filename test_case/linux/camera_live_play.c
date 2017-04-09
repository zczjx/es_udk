/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: camera_live_play.c 
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
#include<es_video.h>
#include<es_display.h>

#define ES_DEBUG 1
#include<es_common.h>
#include<es_media_frame.h>
#include<es_frame_convert.h>

#define BRIGHTNESS_CTRL_ID 0x980900
int main(int argc, char *argv[])
{
	es_error_t ret = ES_SUCCESS;
	es_video_hld v_hld = NULL;
	struct es_video_attr v_attr;
	struct es_media_frame *vframe = NULL;

	es_disp_hld d_hld = NULL;
	struct es_display_attr d_attr;

	convert_hld c_hld = NULL;
	
	
	
	ret = es_video_init();
	check_ret(ret, "es_video_init");

	ret = es_video_open("/dev/video15", ES_VIDEO_CLASS_CAMERA, &v_hld);
	check_ret(ret, "es_video_open");


	ret = es_video_get_attr(v_hld, &v_attr);
	check_ret(ret, "es_video_get_attr");
	printf("[/dev/video15]: property is %d \n", v_attr.property);
	printf("[/dev/video15]: pixel format is %d \n", v_attr.pix_fomat);
	printf("[/dev/video15]: bytes per pixel is %d \n", v_attr.bpp);
	printf("[/dev/video15]: resolution x : %d, y : %d\n", v_attr.resolution.x, v_attr.resolution.y);

	ret = es_video_start(v_hld);
	check_ret(ret, "es_video_start");

	ret = es_display_init();
	check_ret(ret, "es_display_init");

	ret = es_display_open("/dev/fb0", ES_DISPLAY_CLASS_FB, &d_hld);
	check_ret(ret, "es_display_open");

	ret = es_display_get_attr(d_hld, &d_attr);
	check_ret(ret, "es_display_get_attr");

	printf("[/dev/fb*]: pix_fmt is %d \n", d_attr.pix_fmt);
	printf("[/dev/fb*]: x_resolution is %d \n", d_attr.x_resolution);
	printf("[/dev/fb*]: y_resolution is %d \n", d_attr.y_resolution);
	printf("[/dev/fb*]: bpp is %d \n", d_attr.bpp);

	ret = es_frame_convert_init();
	check_ret(ret, "es_frame_convert_init");
	ret = es_frame_convert_open(ES_PIXEL_FRAME, &c_hld);
	check_ret(ret, "es_frame_convert_open ES_PIXEL_FRAME");
	int j = 0;
	while(1)
	{
		es_common_delay(10000);
		vframe = es_media_frame_create();
		ret = es_video_sync_recv_frame(v_hld, vframe);
		check_ret(ret, "es_video_sync_recv_frame");
		
		printf("vframe->type : %d \n", vframe->type);
		printf("vframe->attr.pix_frame.pix_fmt  : %d \n", vframe->attr.pix_frame.pix_fmt );
		printf("vframe->attr.pix_frame.bpp : %d \n", vframe->attr.pix_frame.bpp);
		printf("vframe->attr.pix_frame.x_resolution : %d \n", vframe->attr.pix_frame.x_resolution);
		printf("vframe->attr.pix_frame.y_resolution : %d \n", vframe->attr.pix_frame.y_resolution);
		printf("vframe->buf_size  : %d \n", vframe->buf_size);
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
	
	return 0;
}



