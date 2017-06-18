/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: es_multimedia_format.h 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2017-6-16    
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
#ifndef _ES_MULTIMEDIA_FORMAT_H_
#define _ES_MULTIMEDIA_FORMAT_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <es_common.h>
#include <es_list.h>

typedef unsigned int es_multimedia_fmt_t;

typedef enum{
	ES_PIX_FMT_UNKNOW = 0,

	ES_PIX_FMT_GRAY8,
	ES_PIX_FMT_GRAY16,
		
	ES_PIX_FMT_RGB332,
	ES_PIX_FMT_RGB565,
	ES_PIX_FMT_RGB24,
	ES_PIX_FMT_BGRA32,
	
	ES_PIX_FMT_YUYV,
	
	ES_PIX_FMT_NR,
} es_pix_fmt;

typedef enum{
	ES_IMAGE_ENCODE_FMT_UNKNOW = 0,
		
	ES_IMAGE_ENCODE_FMT_JPEG,
	ES_IMAGE_ENCODE_FMT_PNG,

	ES_IMAGE_ENCODE_FMT_NR,
} es_image_encode_fmt;

typedef enum{
	ES_VIDEO_ENCODE_FMT_UNKNOW = 0,
		
	ES_VIDEO_ENCODE_FMT_MJPEG,
	ES_VIDEO_ENCODE_FMT_H264,

	ES_VIDEO_ENCODE_FMT_NR,
} es_video_encode_fmt;

typedef enum{
	ES_AUDIO_ENCODE_FMT_UNKNOW = 0,
		
	ES_AUDIO_ENCODE_FMT_MP3,
	ES_AUDIO_ENCODE_FMT_AAC,
	ES_AUDIO_ENCODE_FMT_AC3,
	ES_AUDIO_ENCODE_FMT_ADPCM,

	ES_AUDIO_ENCODE_FMT_NR,
} es_audio_encode_fmt;



typedef enum{
	ES_AV_PKG_FMT_UNKNOW = 0,

	ES_AV_PKG_FMT_AVI,
	ES_AV_PKG_FMT_FLV,
	ES_AV_PKG_FMT_MKV,
	ES_AV_PKG_FMT_MP4,
	ES_AV_PKG_FMT_TS,
	
	ES_AV_PKG_FMT_NR,
} es_av_pkg_fmt;


typedef struct es_encode_audio_attr{
	es_audio_encode_fmt encode_fmt;
} es_encode_audio_attr;

typedef struct es_pcm_sound_attr{
	unsigned long stub;
} es_pcm_sound_attr;

typedef struct es_encode_video_attr{
	es_video_encode_fmt encode_fmt;
} es_encode_video_attr;


typedef struct es_encode_image_attr{
	es_image_encode_fmt encode_fmt;
} es_encode_image_attr;

typedef struct es_pixel_frame_attr{
	es_pix_fmt 			pix_fmt;
	unsigned long 		x_resolution; 
	unsigned long 		y_resolution; 
	unsigned long 		bits_per_pix;
} es_pixel_frame_attr;

extern es_multimedia_fmt_t es_pix_fmt_to_v4l2_fmt(es_pix_fmt fmt);
extern es_multimedia_fmt_t es_image_encode_fmt_to_v4l2_fmt(es_image_encode_fmt fmt);
extern es_multimedia_fmt_t es_video_encode_fmt_to_v4l2_fmt(es_video_encode_fmt fmt);

extern es_pix_fmt v4l2_fmt_to_es_pix_fmt(es_multimedia_fmt_t fmt);
extern es_image_encode_fmt v4l2_fmt_to_es_image_encode_fmt(es_multimedia_fmt_t fmt);
extern es_video_encode_fmt v4l2_fmt_to_es_video_encode_fmt(es_multimedia_fmt_t fmt);

extern es_multimedia_fmt_t es_pix_fmt_to_ffmpeg_fmt(es_pix_fmt fmt);
extern es_multimedia_fmt_t es_image_encode_fmt_to_ffmpeg_fmt(es_image_encode_fmt fmt);
extern es_multimedia_fmt_t es_video_encode_fmt_to_ffmpeg_fmt(es_video_encode_fmt fmt);
extern es_multimedia_fmt_t es_audio_encode_fmt_to_ffmpeg_fmt(es_audio_encode_fmt fmt);

extern es_pix_fmt ffmpeg_fmt_to_es_pix_fmt(es_multimedia_fmt_t fmt);
extern es_image_encode_fmt ffmpeg_fmt_to_es_image_encode_fmt(es_multimedia_fmt_t fmt);
extern es_video_encode_fmt ffmpeg_fmt_to_es_video_encode_fmt(es_multimedia_fmt_t fmt);
extern es_audio_encode_fmt ffmpeg_fmt_to_es_audio_encode_fmt(es_multimedia_fmt_t fmt);

extern unsigned long es_pix_fmt_bits_per_pixel(es_pix_fmt fmt);
extern unsigned long es_pix_fmt_bytes_per_pixel(es_pix_fmt fmt);
#ifdef __cplusplus
}
#endif

#endif /* ifndef _ES_MULTIMEDIA_FORMAT_H_.2017-6-16 23:06:55 zcz */

