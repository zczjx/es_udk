/*******************************************************************************
* Copyright (C), 2000-2018,  Electronic Technology Co., Ltd.
*                
* @filename: es_codec.h 
*                
* @author: Clarence.Zhou <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2018-1-9    
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
#ifndef _ES_CODEC_H_
#define _ES_CODEC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <es_list.h>
#include <es_common.h>
#include <es_data_frame.h>
#include <es_data_chunk.h>
#include <es_multimedia_format.h>

typedef void * es_codec_hld;

typedef enum {
	ES_CODEC_CLASS_UNKNOW = 0,

	ES_CODEC_CLASS_AUDIO,

	ES_CODEC_CLASS_VIDEO,

	ES_CODEC_CLASS_IMAGE,
	
} es_codec_class;

typedef union es_codec_fmt {
	es_audio_encode_fmt audio_fmt; 
	es_video_encode_fmt video_fmt;
	es_image_encode_fmt image_fmt;
} es_codec_fmt;

#define ES_CODEC_CAP_UNKNOW					(0)
#define ES_CODEC_CAP_FFMPEG_ENCODE			(1 <<  1)
#define ES_CODEC_CAP_FFMPEG_DECODE			(1 <<  2)
#define ES_CODEC_CAP_DEV_HARDWARE_ENCODE	(1 <<  3)
#define ES_CODEC_CAP_DEV_HARDWARE_DECODE	(1 <<  4)


typedef struct es_codec_attr {
	es_codec_class codec_class;
	union es_codec_fmt codec_fmt;
	long codec_cap;
	char *codec_path; /* optional */
} es_codec_attr;


extern es_error_t es_codec_init();

extern es_error_t es_codec_deinit();

extern es_error_t es_codec_open(struct es_codec_attr *c_attr, es_codec_hld *c_hld);

extern es_error_t es_codec_close(es_codec_hld c_hld);

extern es_error_t es_codec_get_attr(es_codec_hld c_hld, struct es_codec_attr *c_attr);

extern es_error_t es_codec_decode(es_codec_hld c_hld, struct es_data_chunk *input_dat,
	struct es_data_frame **out_frame_list, int *frame_count);

extern es_error_t es_codec_encode(es_codec_hld c_hld, struct es_data_frame *input_dat,
	struct es_data_chunk *output_dat, int *got_enc_chunk);


#ifdef __cplusplus
}
#endif

#endif /* ifndef _ES_CODEC_H_.2018-1-9 22:36:19 zcz */

