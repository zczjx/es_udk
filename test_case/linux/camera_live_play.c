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
#include<time.h>
#include<es_video.h>
#include<es_display.h>

#define ES_DEBUG 1
#include<es_common.h>
#include<es_data_frame.h>
#include<es_data_chunk.h>
#include<es_frame_convert.h>
#include<es_codec.h>

#define BRIGHTNESS_CTRL_ID 0x980900
int main(int argc, char *argv[])
{
	es_error_t ret = ES_SUCCESS;
	es_video_hld v_hld = NULL;
	struct es_video_attr v_attr;
	struct es_data_frame *vframe = NULL;
	struct es_data_chunk *vchunk = NULL;
	struct timespec tp;
	static long pre_time = 0;
	static long curr_time = 0;
	static long fps = 0;
	
	es_disp_hld d_hld = NULL;
	struct es_display_attr d_attr;

	convert_hld c_hld = NULL;

	es_codec_hld codec_hld = NULL;
	struct es_codec_attr codec_attr;
	
	
	
	ret = es_video_init();
	check_ret(ret, "es_video_init");

	ret = es_video_open("/dev/video0", ES_VIDEO_CLASS_CAMERA, &v_hld);
	check_ret(ret, "es_video_open");


	ret = es_video_get_attr(v_hld, &v_attr);
	check_ret(ret, "es_video_get_attr");
	printf("[/dev/video0]: property is %d \n", v_attr.property);
	printf("[/dev/video0]: video_data_type %d \n", v_attr.video_data_type);
	printf("[/dev/video0]: pixel format is %d \n", v_attr.dat_fmt.pix_fmt);
	printf("[/dev/video0]: store_fmt is %d \n", v_attr.pix_fmt_info.store_fmt);
	printf("[/dev/video0]: bits per pixel is %d \n", v_attr.pix_fmt_info.bits_per_pix);
	printf("[/dev/video0]: resolution x : %d, y : %d\n", v_attr.resolution.x, v_attr.resolution.y);

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
	printf("[/dev/fb*]: store_fmt is %d \n", d_attr.pix_fmt_info.store_fmt);
	printf("[/dev/fb*]: bpp is %d \n", d_attr.pix_fmt_info.bits_per_pix);

	ret = es_frame_convert_init();
	check_ret(ret, "es_frame_convert_init");
	ret = es_frame_convert_open(ES_PIXEL_FRAME, &c_hld);
	check_ret(ret, "es_frame_convert_open ES_PIXEL_FRAME");
	int j = 0;

	ret = es_codec_init();
	check_ret(ret, "es_codec_init");
	codec_attr.codec_class = ES_CODEC_CLASS_VIDEO;
	codec_attr.codec_fmt.video_fmt = ES_VIDEO_ENCODE_FMT_MJPEG;
	codec_attr.codec_cap = ES_CODEC_CAP_FFMPEG_DECODE;
	ret = es_codec_open(&codec_attr, &codec_hld);
	check_ret(ret, "es_codec_open");
	
	while(1)
	{
		clock_gettime(CLOCK_MONOTONIC, &tp);
		curr_time = tp.tv_sec;
		fps++;
		if((curr_time - pre_time) >= 1)
		{
			printf("current fps is %d \n", fps);
			pre_time = curr_time;
			fps = 0;
		}
		switch(v_attr.video_data_type)
		{
			case ES_VIDEO_DATA_TYPE_PIXEL_FRAME:
				vframe = es_data_frame_create();
				ret = es_video_sync_recv_frame(v_hld, vframe);
				check_ret(ret, "es_video_sync_recv_frame");
		
				printf("vframe->type : %d \n", vframe->type);
				printf("vframe->attr.pix_frame.pix_fmt  : %d \n", vframe->frame_attr.pix_frame.pix_fmt);
				printf("vframe->frame_attr.pix_frame.pix_fmt_info.bits_per_pix : %d \n", vframe->frame_attr.pix_frame.pix_fmt_info.bits_per_pix);
				printf("vframe->frame_attr.pix_frame.pix_fmt_info.store_fmt : %d \n", vframe->frame_attr.pix_frame.pix_fmt_info.store_fmt);
				printf("vframe->attr.pix_frame.x_resolution : %d \n", vframe->frame_attr.pix_frame.x_resolution);
				printf("vframe->attr.pix_frame.y_resolution : %d \n", vframe->frame_attr.pix_frame.y_resolution);
				printf("vframe->buf_size  : %d \n", vframe->buf_size);
				if(vframe->frame_attr.pix_frame.pix_fmt != d_attr.pix_fmt)
				{
					struct es_data_frame *dframe = NULL;
					dframe = es_convert_to_spec_frame_fmt(c_hld, vframe, d_attr.pix_fmt);
					if(NULL != dframe)
					{
						ret = es_display_sync_flush(d_hld, dframe);
						check_ret(ret, "es_display_sync_flush");
						es_data_frame_destroy(dframe);
						es_data_frame_destroy(vframe);
					}
					else
					{
						es_data_frame_destroy(vframe);
					}

				}
				else
				{
					vframe->frame_attr.pix_frame.pix_fmt_info.store_fmt = ES_PIX_STORE_FMT_PACKED;
					vframe->frame_attr.pix_frame.pix_fmt_info.bits_per_pix = 32;
					vframe->frame_attr.pix_frame.pix_fmt = ES_PIX_FMT_BGRA32;
					es_display_sync_flush(d_hld, vframe);
					es_data_frame_destroy(vframe);
				}
				break;

			case ES_VIDEO_DATA_TYPE_ENCODE_VIDEO_CHUNK:
			{
				struct es_data_frame *pix_frame_list = NULL;
				int frame_count = 0;
					
				vchunk = es_data_chunk_create();
				ret = es_video_sync_recv_encode_chunk(v_hld, vchunk);
				check_ret(ret, "es_video_sync_recv_encode_chunk");
				printf("vchunk->type: %d \n", vchunk->type);
				printf("vchunk->chunk_attr.encode_video.encode_fmt: %d \n", vchunk->chunk_attr.encode_video.encode_fmt);
				printf("vchunk->buf_size: %d \n", vchunk->buf_size);
				es_codec_decode(codec_hld, vchunk, &pix_frame_list, &frame_count);
				printf("decode frame_count: %d \n", frame_count);
				pix_frame_list->frame_attr.pix_frame.pix_fmt_info.bits_per_pix  = 32;
				pix_frame_list->frame_attr.pix_frame.pix_fmt_info.store_fmt  = ES_PIX_STORE_FMT_PACKED;
				pix_frame_list->frame_attr.pix_frame.pix_fmt = ES_PIX_FMT_BGRA32;
				es_display_sync_flush(d_hld, pix_frame_list);
				es_data_frame_destroy(pix_frame_list);
				pix_frame_list = NULL;
			
				break;
			}
			default:
				
				break;
		}
		
	}
	
	return 0;
}



