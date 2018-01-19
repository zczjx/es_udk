/*******************************************************************************
* Copyright (C), 2000-2018,  Electronic Technology Co., Ltd.
*                
* @filename: ffmpeg_video_codec.c 
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

typedef struct ffmpeg_codec_desc {
	int ffmpeg_codec_id;
	AVCodec *codec;
    AVCodecContext *ctx;
    AVFrame *frame;	
	int frame_total_count;
    AVPacket avpkt;
} ffmpeg_codec_desc;

/**********************decode func decl***********************************/
typedef es_error_t (*spec_decode_func)(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count);

static es_error_t default_vdecode_func(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count);

static es_error_t mjpeg_vdecode_func(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count);

static spec_decode_func ffmpeg_decode_func_arr[] = {
	[ES_VIDEO_ENCODE_FMT_UNKNOW] = NULL,
		
	[ES_VIDEO_ENCODE_FMT_MJPEG] = mjpeg_vdecode_func,
	[ES_VIDEO_ENCODE_FMT_H264] = default_vdecode_func,
};

/****************************************************/


static bool is_ffmpeg_vcodec_support(es_codec_class codec_class, 
	union es_codec_fmt codec_fmt, long codec_cap);

	
static es_error_t ffmpeg_vcodec_open(struct codec_base *base,
	struct es_codec_attr *c_attr);
	
static es_error_t ffmpeg_vcodec_close(struct codec_base *base);
	
static es_error_t ffmpeg_vcodec_decode(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count);

static es_error_t ffmpeg_vcodec_encode(struct codec_base *base, 
	struct es_data_frame *input_dat, struct es_data_chunk *output_dat, 
	int *got_chunk);



/*******************************************************************************
* @function name: is_ffmpeg_vcodec_support    
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
static bool is_ffmpeg_vcodec_support(es_codec_class codec_class, 
	union es_codec_fmt codec_fmt, long codec_cap)
{
	bool ret = es_false;
	long temp = 0;
	es_multimedia_fmt_t ffmpeg_vfmt = AV_CODEC_ID_NONE;
	AVCodec *codec;

	if((ES_CODEC_CLASS_VIDEO != codec_class) 
	|| (ES_CODEC_CAP_UNKNOW == codec_cap))
		return es_false;

	temp = codec_cap & (~(ES_CODEC_CAP_FFMPEG_ENCODE | ES_CODEC_CAP_FFMPEG_DECODE));
	if(0 != temp) /* have another exclude ffmpeg cap*/
		return es_false;

	ffmpeg_vfmt = es_video_encode_fmt_to_ffmpeg_fmt(codec_fmt.video_fmt);
	if(AV_CODEC_ID_NONE == ffmpeg_vfmt)
		return es_false;
	
	if(codec_cap & ES_CODEC_CAP_FFMPEG_DECODE)
	{
		ret = es_true;
		codec = avcodec_find_decoder(ffmpeg_vfmt);
		if(!codec)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("Decoder not found\n");
			ret = es_false;
			goto out;
		}
	}
	
	if(codec_cap & ES_CODEC_CAP_FFMPEG_ENCODE)
	{
		ret = es_true;
		codec = avcodec_find_encoder(ffmpeg_vfmt);
		if(!codec)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("Encoder not found\n");
			ret = es_false;
			goto out;
		}
	}

out:	
	return ret;
}
	

/*******************************************************************************
* @function name: ffmpeg_vcodec_open    
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
static es_error_t ffmpeg_vcodec_open(struct codec_base *base,
	struct es_codec_attr *c_attr)
{
	es_error_t ret = ES_SUCCESS;
	struct ffmpeg_codec_desc *codec_desc = NULL;

	if((NULL == base) || (NULL == c_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	if(ES_CODEC_CLASS_VIDEO != c_attr->codec_class)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}

	if(c_attr->codec_cap & ES_CODEC_CAP_FFMPEG_DECODE)
	{
		codec_desc = (struct ffmpeg_codec_desc *) malloc(sizeof(struct ffmpeg_codec_desc));
		memset(codec_desc, 0x00, sizeof(struct ffmpeg_codec_desc));
		av_init_packet(&codec_desc->avpkt);
		codec_desc->ffmpeg_codec_id  = es_video_encode_fmt_to_ffmpeg_fmt(c_attr->codec_fmt.video_fmt);
		if(AV_CODEC_ID_NONE == codec_desc->ffmpeg_codec_id)
		{
			ret = ES_INVALID_PARAM;
			goto err_out;
		}

    	/* find the video decoder */
    	codec_desc->codec = avcodec_find_decoder(codec_desc->ffmpeg_codec_id);
    	if (!codec_desc->codec)
		{
        	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("Decoder not found\n");;
        	ret = ES_INVALID_PARAM;
			goto err_out;
    	}

    	codec_desc->ctx = avcodec_alloc_context3(codec_desc->codec);
    	if (!codec_desc->ctx)
		{
        	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("Could not allocate video decode context\n");;
        	ret = ES_INVALID_PARAM;
			goto err_out;
    	}

    	if (codec_desc->codec->capabilities & AV_CODEC_CAP_TRUNCATED)
        	codec_desc->ctx->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames

	    /* For some codecs, such as msmpeg4 and mpeg4, width and height
	       MUST be initialized there because this information is not
	       available in the bitstream. */

    	/* open it */
    	if (avcodec_open2(codec_desc->ctx, codec_desc->codec, NULL) < 0) 
		{
        	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("Could not open decode\n");
        	ret = ES_INVALID_PARAM;
			goto err_out;
    	}

    	codec_desc->frame = av_frame_alloc();
    	if (!codec_desc->frame) 
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("Could not allocate video frame\n");
        	ret = ES_INVALID_PARAM;
			goto err_out;
    	}

    	codec_desc->frame_total_count = 0;
		base->decode_priv_desc = codec_desc;
		codec_desc = NULL;
	}

	if(c_attr->codec_cap & ES_CODEC_CAP_FFMPEG_ENCODE)
	{
		ret = ES_INVALID_PARAM;
		goto err_out;
	}

	base->sub_class = ES_CODEC_CLASS_VIDEO;
	base->attr.codec_class = ES_CODEC_CLASS_VIDEO;
	base->attr.codec_fmt.video_fmt = c_attr->codec_fmt.video_fmt;
	base->attr.codec_cap = c_attr->codec_cap & (ES_CODEC_CAP_FFMPEG_ENCODE | ES_CODEC_CAP_FFMPEG_DECODE);
	base->attr.codec_path = NULL;
	return ret;

err_out:
	if(NULL != codec_desc)
	{
		if(NULL != codec_desc->ctx)
		{
			avcodec_close(codec_desc->ctx);
			av_free(codec_desc->ctx);
			codec_desc->ctx = NULL;

		}

		if(NULL != codec_desc->frame)
		{
			av_frame_free(&codec_desc->frame);
			codec_desc->frame = NULL;
		}
		free(codec_desc);
		codec_desc = NULL;
	}
	
	if(NULL != base->decode_priv_desc)
	{
		codec_desc = base->decode_priv_desc;
		if(NULL != codec_desc->ctx)
		{
			avcodec_close(codec_desc->ctx);
			av_free(codec_desc->ctx);
			codec_desc->ctx = NULL;
		}

		if(NULL != codec_desc->frame)
		{
			av_frame_free(&codec_desc->frame);
			codec_desc->frame = NULL;
		}
		free(codec_desc);
		codec_desc = NULL;
		base->decode_priv_desc= NULL;
	}
	
	if(NULL != base->encode_priv_desc)
	{
		codec_desc = base->encode_priv_desc;
		if(NULL != codec_desc->ctx)
		{
			avcodec_close(codec_desc->ctx);
			av_free(codec_desc->ctx);
			codec_desc->ctx = NULL;
		}

		if(NULL != codec_desc->frame)
		{
			av_frame_free(&codec_desc->frame);
			codec_desc->frame = NULL;
		}
		free(codec_desc);
		codec_desc = NULL;
		base->encode_priv_desc= NULL;
	}
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
static es_error_t ffmpeg_vcodec_close(struct codec_base *base)
{
	es_error_t ret = ES_SUCCESS;
	struct ffmpeg_codec_desc *codec_desc = NULL;

	if(NULL == base)
		return ES_SUCCESS;

	if(NULL != base->decode_priv_desc)
	{
		codec_desc = base->decode_priv_desc;
		if(NULL != codec_desc->ctx)
		{
			avcodec_close(codec_desc->ctx);
			av_free(codec_desc->ctx);
			codec_desc->ctx = NULL;
		}

		if(NULL != codec_desc->frame)
		{
			av_frame_free(&codec_desc->frame);
			codec_desc->frame = NULL;
		}
		free(codec_desc);
		codec_desc = NULL;
		base->decode_priv_desc= NULL;
	}
	
	if(NULL != base->encode_priv_desc)
	{
		codec_desc = base->encode_priv_desc;
		if(NULL != codec_desc->ctx)
		{
			avcodec_close(codec_desc->ctx);
			av_free(codec_desc->ctx);
			codec_desc->ctx = NULL;
		}

		if(NULL != codec_desc->frame)
		{
			av_frame_free(&codec_desc->frame);
			codec_desc->frame = NULL;
		}
		free(codec_desc);
		codec_desc = NULL;
		base->encode_priv_desc= NULL;
	}

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
static es_error_t ffmpeg_vcodec_decode(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count)
{
	es_error_t ret = ES_SUCCESS;
	struct ffmpeg_codec_desc *codec_desc = NULL;
	int len, got_frame;
	spec_decode_func decode_func = NULL;

	if((NULL == base) || (NULL == input_dat)
	|| (NULL == out_frame_list) || (NULL == frame_count))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}

	if( (base->attr.codec_fmt.video_fmt >= ES_VIDEO_ENCODE_FMT_NR)
	|| (base->attr.codec_fmt.video_fmt <= ES_VIDEO_ENCODE_FMT_UNKNOW) )
	{
		*frame_count = 0;
		*out_frame_list = NULL;
		return ES_INVALID_PARAM;
	}

	decode_func = ffmpeg_decode_func_arr[base->attr.codec_fmt.video_fmt];
	return decode_func(base, input_dat, out_frame_list, frame_count);
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
static es_error_t ffmpeg_vcodec_encode(struct codec_base *base, 
	struct es_data_frame *input_dat, struct es_data_chunk *output_dat, 
	int *got_chunk)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}


/*******************************************************************************
* @function name: default_vdecode_func    
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
static es_error_t default_vdecode_func(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count)
{
	es_error_t ret = ES_SUCCESS;
	struct ffmpeg_codec_desc *codec_desc = NULL;
	int i, len, got_frame;

	if((NULL == base) || (NULL == input_dat)
	|| (NULL == out_frame_list) || (NULL == frame_count))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	
	codec_desc = base->decode_priv_desc;
	*frame_count = 0;
	*out_frame_list = NULL;
	if(NULL == codec_desc)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}

	if(input_dat->buf_size <= 0)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no dat to decode\n" , __FUNCTION__);
        return ES_SUCCESS;
	}
	
	if((ES_ENCODE_VIDEO_CHUNK != input_dat->type)
	|| (base->attr.codec_fmt.video_fmt != input_dat->chunk_attr.encode_video.encode_fmt))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("warning: [%s] input dat chunk format do not match decode format!\n" , __FUNCTION__);
	}
	
	codec_desc->avpkt.size = input_dat->buf_size;
	codec_desc->avpkt.data = input_dat->buf_start_addr;
	while (codec_desc->avpkt.size > 0)
	{
    	len = avcodec_decode_video2(codec_desc->ctx, codec_desc->frame, 
									&got_frame, &codec_desc->avpkt);

		ES_PRINTF("avcodec_decode_video2 len %d got_frame: %d\n", len, got_frame);
    	if (len < 0) 
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Error while decoding frame %d\n" , __FUNCTION__, *frame_count);
        	return ES_FAIL;
    	}
    	if (got_frame) 
		{
			struct es_data_frame *vframe = NULL;
			int frame_bytes = 0;
			struct es_list_head *frame_head = NULL;

			ES_PRINTF("ffmpeg_vcodec_decode got_frame %d\n", *frame_count);
			vframe = es_data_frame_create();
			if(NULL == vframe)
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] es_data_frame_create no mem to create!\n" , __FUNCTION__);
        		return ES_FAIL;
			}
			
			vframe->type = ES_PIXEL_FRAME;
			vframe->frame_attr.pix_frame.pix_fmt = ffmpeg_fmt_to_es_pix_fmt(codec_desc->frame->format);
			es_get_pix_fmt_info(vframe->frame_attr.pix_frame.pix_fmt, 
				&vframe->frame_attr.pix_frame.pix_fmt_info);
			vframe->frame_attr.pix_frame.x_resolution = codec_desc->frame->width;
			vframe->frame_attr.pix_frame.y_resolution = codec_desc->frame->height;
			frame_bytes = codec_desc->frame->width 
						* codec_desc->frame->height 
						* (vframe->frame_attr.pix_frame.pix_fmt_info.bits_per_pix >> 3);
			ret = es_data_frame_buf_alloc(vframe, DATA_FRAME_MEM_METHOD_MALLOC, frame_bytes); 

			switch(vframe->frame_attr.pix_frame.pix_fmt_info.store_fmt)
			{
				case ES_PIX_STORE_FMT_PACKED:
					memcpy(vframe->buf_start_addr, codec_desc->frame->data[0], frame_bytes);
					for(i = 0; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
					{
						vframe->planes[i] = NULL;
						vframe->plane_bytes[i] = 0;
					}
					break;
					
				case ES_PIX_STORE_FMT_PLANAR:
					vframe->planes[0] = vframe->buf_start_addr;
					vframe->plane_bytes[0] = codec_desc->frame->linesize[0] * codec_desc->frame->height;
					memcpy(vframe->planes[0], codec_desc->frame->data[0], vframe->plane_bytes[0]);		
					for(i = 1; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
					{
						if(0 == codec_desc->frame->linesize[i])
							break;
						
						vframe->plane_bytes[i] = codec_desc->frame->linesize[i] * codec_desc->frame->height;
						vframe->planes[i] = vframe->planes[i-1] 
											+ vframe->plane_bytes[i-1];
						memcpy(vframe->planes[i], codec_desc->frame->data[i], vframe->plane_bytes[i]);
						
					}
					break;
					
				case ES_PIX_STORE_FMT_SEMI_PLANAR:

					break;
				default:

					break;
			}

			if(NULL == (*out_frame_list))
			{
				 /* frame list head */
				 *out_frame_list = vframe;
				 vframe = NULL;
			}
			else
			{
				frame_head = &(*out_frame_list)->entry;
				es_list_add_tail(&vframe->entry, frame_head);
				vframe = NULL;
			}
        	(*frame_count)++;
    	}
    	if (codec_desc->avpkt.data) 
		{
        	codec_desc->avpkt.size -= len;
        	codec_desc->avpkt.data += len;
    	}
	}

    /* Some codecs, such as MPEG, transmit the I- and P-frame with a
       latency of one frame. You must do the following to have a
       chance to get the last frame of the video. */
    codec_desc->avpkt.data = NULL;
    codec_desc->avpkt.size = 0;

    len = avcodec_decode_video2(codec_desc->ctx, codec_desc->frame, 
								&got_frame, &codec_desc->avpkt);
    if (len < 0) 
	{
		ret = ES_SUCCESS;
        goto end;
    }
    if (got_frame) 
	{
		struct es_data_frame *vframe = NULL;
		int frame_bytes = 0;
		struct es_list_head *frame_head = NULL;

		vframe = es_data_frame_create();
		if(NULL == vframe)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] es_data_frame_create no mem to create!\n" , __FUNCTION__);
        	return ES_FAIL;
		}
			
		vframe->type = ES_PIXEL_FRAME;
		vframe->frame_attr.pix_frame.pix_fmt = ffmpeg_fmt_to_es_pix_fmt(codec_desc->frame->format);
		es_get_pix_fmt_info(vframe->frame_attr.pix_frame.pix_fmt, 
			&vframe->frame_attr.pix_frame.pix_fmt_info);
		vframe->frame_attr.pix_frame.x_resolution = codec_desc->frame->width;
		vframe->frame_attr.pix_frame.y_resolution = codec_desc->frame->height;
		frame_bytes = codec_desc->frame->width 
					* codec_desc->frame->height 
					* (vframe->frame_attr.pix_frame.pix_fmt_info.bits_per_pix >> 3);
		ret = es_data_frame_buf_alloc(vframe, DATA_FRAME_MEM_METHOD_MALLOC, frame_bytes); 

		switch(vframe->frame_attr.pix_frame.pix_fmt_info.store_fmt)
		{
			case ES_PIX_STORE_FMT_PACKED:
				memcpy(vframe->buf_start_addr, codec_desc->frame->data[0], frame_bytes);
				for(i = 0; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
				{
					vframe->planes[i] = NULL;
					vframe->plane_bytes[i] = 0;
				}
				break;
				
			case ES_PIX_STORE_FMT_PLANAR:
				vframe->planes[0] = vframe->buf_start_addr;
				vframe->plane_bytes[0] = codec_desc->frame->linesize[0] * codec_desc->frame->height;
				memcpy(vframe->planes[0], codec_desc->frame->data[0], vframe->plane_bytes[0]);		
				for(i = 1; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
				{
					if(0 == codec_desc->frame->linesize[i])
						break;
						
					vframe->plane_bytes[i] = codec_desc->frame->linesize[i] * codec_desc->frame->height;
					vframe->planes[i] = vframe->planes[i-1] 
											+ vframe->plane_bytes[i-1];
					memcpy(vframe->planes[i], codec_desc->frame->data[i], vframe->plane_bytes[i]);
						
				}
				break;
				
			case ES_PIX_STORE_FMT_SEMI_PLANAR:

				break;
			default:

				break;
		}
		
		if(NULL == (*out_frame_list))
		{
			/* frame list head */
			*out_frame_list = vframe;
			vframe = NULL;
		}
		else
		{
			frame_head = &(*out_frame_list)->entry;
			es_list_add_tail(&vframe->entry, frame_head);
			vframe = NULL;
		}
        (*frame_count)++;
    }
  	ret = ES_SUCCESS;
end:
	return ret;

}

/*******************************************************************************
* @function name: mjpeg_vdecode_func    
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
static es_error_t mjpeg_vdecode_func(struct codec_base *base, 
	struct es_data_chunk *input_dat, struct es_data_frame **out_frame_list, 
	int *frame_count)
{
	es_error_t ret = ES_SUCCESS;
	struct ffmpeg_codec_desc *codec_desc = NULL;
	int i, len, got_frame;

	if((NULL == base) || (NULL == input_dat)
	|| (NULL == out_frame_list) || (NULL == frame_count))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	
	codec_desc = base->decode_priv_desc;
	*frame_count = 0;
	*out_frame_list = NULL;
	if(NULL == codec_desc)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}

	if(input_dat->buf_size <= 0)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no dat to decode\n" , __FUNCTION__);
        return ES_SUCCESS;
	}
	
	if((ES_ENCODE_VIDEO_CHUNK != input_dat->type)
	|| (base->attr.codec_fmt.video_fmt != input_dat->chunk_attr.encode_video.encode_fmt))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("warning: [%s] input dat chunk format do not match decode format!\n" , __FUNCTION__);
	}
	
	codec_desc->avpkt.size = input_dat->buf_size;
	codec_desc->avpkt.data = input_dat->buf_start_addr;
    len = avcodec_decode_video2(codec_desc->ctx, codec_desc->frame, 
								&got_frame, &codec_desc->avpkt);

    if (len < 0) 
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Error while decoding frame %d\n" , __FUNCTION__, *frame_count);
        return ES_FAIL;
    }
    if (got_frame) 
	{
			struct es_data_frame *vframe = NULL;
			int frame_bytes = 0;
			struct es_list_head *frame_head = NULL;

			ES_PRINTF("ffmpeg_vcodec_decode got_frame %d\n", *frame_count);
			vframe = es_data_frame_create();
			if(NULL == vframe)
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] es_data_frame_create no mem to create!\n" , __FUNCTION__);
        		return ES_FAIL;
			}
			
			vframe->type = ES_PIXEL_FRAME;
			vframe->frame_attr.pix_frame.pix_fmt = ffmpeg_fmt_to_es_pix_fmt(codec_desc->frame->format);
			es_get_pix_fmt_info(vframe->frame_attr.pix_frame.pix_fmt, 
				&vframe->frame_attr.pix_frame.pix_fmt_info);
			vframe->frame_attr.pix_frame.x_resolution = codec_desc->frame->width;
			vframe->frame_attr.pix_frame.y_resolution = codec_desc->frame->height;
			frame_bytes = codec_desc->frame->width 
						* codec_desc->frame->height 
						* (vframe->frame_attr.pix_frame.pix_fmt_info.bits_per_pix >> 3);
			ret = es_data_frame_buf_alloc(vframe, DATA_FRAME_MEM_METHOD_MALLOC, frame_bytes); 

			switch(vframe->frame_attr.pix_frame.pix_fmt_info.store_fmt)
			{
				case ES_PIX_STORE_FMT_PACKED:
					memcpy(vframe->buf_start_addr, codec_desc->frame->data[0], frame_bytes);
					for(i = 0; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
					{
						vframe->planes[i] = NULL;
						vframe->plane_bytes[i] = 0;
					}
					break;
					
				case ES_PIX_STORE_FMT_PLANAR:
					vframe->planes[0] = vframe->buf_start_addr;
					vframe->plane_bytes[0] = codec_desc->frame->linesize[0] * codec_desc->frame->height;
					memcpy(vframe->planes[0], codec_desc->frame->data[0], vframe->plane_bytes[0]);		
					for(i = 1; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
					{
						if(0 == codec_desc->frame->linesize[i])
							break;
						
						vframe->plane_bytes[i] = codec_desc->frame->linesize[i] * codec_desc->frame->height;
						vframe->planes[i] = vframe->planes[i-1] 
											+ vframe->plane_bytes[i-1];
						memcpy(vframe->planes[i], codec_desc->frame->data[i], vframe->plane_bytes[i]);
						
					}
					break;
					
				case ES_PIX_STORE_FMT_SEMI_PLANAR:

					break;
					
				default:

					break;
			}

			if(NULL == (*out_frame_list))
			{
				 /* frame list head */
				 *out_frame_list = vframe;
				 vframe = NULL;
			}
			else
			{
				frame_head = &(*out_frame_list)->entry;
				es_list_add_tail(&vframe->entry, frame_head);
				vframe = NULL;
			}
        	(*frame_count)++;
    	}
    codec_desc->avpkt.data = NULL;
    codec_desc->avpkt.size = 0;

  	ret = ES_SUCCESS;
	return ret;

}



static struct codec_base ffmpeg_video_codec = {
	.sub_class = ES_CODEC_CLASS_VIDEO,
	.attr = {
		.codec_class = ES_CODEC_CLASS_VIDEO,
		.codec_fmt.video_fmt = ES_VIDEO_ENCODE_FMT_UNKNOW,
		.codec_cap = ES_CODEC_CAP_UNKNOW,
		.codec_path = NULL, /* optional */
	},
	.priv = NULL,
	.decode_priv_desc = NULL,
	.encode_priv_desc = NULL,
	
	.is_codec_support = is_ffmpeg_vcodec_support,
	
	.codec_open = ffmpeg_vcodec_open,
	
	.codec_close = ffmpeg_vcodec_close,
	
	.codec_decode = ffmpeg_vcodec_decode,

	.codec_encode = ffmpeg_vcodec_encode,
	
}; 

es_error_t es_init_ffmpeg_video_codec()
{
	es_error_t ret = ES_SUCCESS;

	avcodec_register_all();
	ret = codec_base_register(&ffmpeg_video_codec);
	return ret;

}

