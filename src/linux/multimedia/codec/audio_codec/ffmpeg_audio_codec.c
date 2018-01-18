/*******************************************************************************
* Copyright (C), 2000-2018,  Electronic Technology Co., Ltd.
*                
* @filename: ffmpeg_audio_codec.c 
*                
* @author: Clarence.Zhou <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2018-1-13    
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


#include <es_codec.h>
#include "../codec_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* ffmpeg header*/
#include <math.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>


static bool is_ffmpeg_codec_support(es_codec_class codec_class, 
	union es_codec_fmt codec_fmt, long codec_cap);

	
static es_error_t ffmpeg_codec_open(struct codec_base *base,
	struct es_codec_attr *c_attr);
	
static es_error_t ffmpeg_codec_close(struct codec_base *base);
	
static es_error_t ffmpeg_codec_decode(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count);

static es_error_t ffmpeg_codec_encode(struct codec_base *base, 
	struct es_data_frame *input_dat, struct es_data_chunk *output_dat, 
	int *got_chunk);



/*******************************************************************************
* @function name: is_ffmpeg_codec_support    
*                
* @brief:          
*                
* @param:        
*                
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
static bool is_ffmpeg_codec_support(es_codec_class codec_class, 
	union es_codec_fmt codec_fmt, long codec_cap)
{
	bool ret = es_false;

	return es_false;

}
	
/*******************************************************************************
* @function name: ffmpeg_codec_open    
*                
* @brief:          
*                
* @param:        
*                
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
static es_error_t ffmpeg_codec_open(struct codec_base *base,
	struct es_codec_attr *c_attr)
{
	es_error_t ret = ES_SUCCESS;

	return ret;


}
	
/*******************************************************************************
* @function name: ffmpeg_codec_close    
*                
* @brief:          
*                
* @param:        
*                
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
static es_error_t ffmpeg_codec_close(struct codec_base *base)
{
	es_error_t ret = ES_SUCCESS;

	return ret;


}
	
/*******************************************************************************
* @function name: ffmpeg_codec_decode    
*                
* @brief:          
*                
* @param:        
*                
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
static es_error_t ffmpeg_codec_decode(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}

/*******************************************************************************
* @function name: codec_encode    
*                
* @brief:          
*                
* @param:        
*                
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
static es_error_t ffmpeg_codec_encode(struct codec_base *base, 
	struct es_data_frame *input_dat, struct es_data_chunk *output_dat, 
	int *got_chunk)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}



static struct codec_base ffmpeg_audio_codec = {
	.sub_class = ES_CODEC_CLASS_AUDIO,
	.attr = {
		.codec_class = ES_CODEC_CLASS_AUDIO,
		.codec_fmt.audio_fmt = ES_VIDEO_ENCODE_FMT_UNKNOW,
		.codec_cap = ES_CODEC_CAP_UNKNOW,
		.codec_path = NULL, /* optional */
	},
	.priv = NULL,
	
	.is_codec_support = is_ffmpeg_codec_support,
	
	.codec_open = ffmpeg_codec_open,
	
	.codec_close = ffmpeg_codec_close,
	
	.codec_decode = ffmpeg_codec_decode,

	.codec_encode = ffmpeg_codec_encode,
	
}; 


es_error_t es_init_ffmpeg_audio_codec()
{
	es_error_t ret = ES_SUCCESS;
	
	ret = codec_base_register(&ffmpeg_audio_codec);
	return ret;

}





