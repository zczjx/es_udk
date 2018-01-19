/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: pixel_frame_convert.c 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2017-1-3    
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
#include <es_frame_convert.h>
#include <es_data_frame.h>
#include "frame_convert_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* ffmpeg header*/
#include <libswscale/swscale.h>  
#include <libavutil/pixfmt.h> 
#include <libavutil/imgutils.h> 

#define SWSCALE_PLANES_NR (4)


typedef struct pframe_convert_priv_attr{
	unsigned char dummy;
} pframe_convert_priv_attr;


static es_error_t do_pix_fmt_convert(es_pix_fmt es_src_fmt, struct es_data_frame *src_frame, 
		es_pix_fmt es_dst_fmt, struct es_data_frame *dst_frame);

/*public func */
static es_error_t pix_frame_convert_open(struct frame_convert_base *base);
static es_error_t pix_frame_convert_close(struct frame_convert_base *base);
static struct es_data_frame * convert_to_spec_pix_frame_fmt(struct frame_convert_base *base,
		struct es_data_frame *src_frame, es_multimedia_fmt_t fmt);




/*******************************************************************************
* @function name: pix_frame_convert_open    
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
static es_error_t pix_frame_convert_open(struct frame_convert_base *base)
{
	es_error_t ret = ES_SUCCESS; 
	struct pframe_convert_priv_attr *priv_attr = NULL;

	if(NULL == base)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	
	base->type = ES_PIXEL_FRAME;
	INIT_ES_LIST_HEAD(&base->entry);
	base->priv = priv_attr;
	return ret;
}

/*******************************************************************************
* @function name: pix_frame_convert_close    
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
static es_error_t pix_frame_convert_close(struct frame_convert_base *base)
{
	es_error_t ret = ES_SUCCESS;

	if(NULL == base)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	return ret;
}

/*******************************************************************************
* @function name: convert_to_spec_pix_frame_fmt    
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
static struct es_data_frame * convert_to_spec_pix_frame_fmt(struct frame_convert_base *base,
		struct es_data_frame *src_frame, es_multimedia_fmt_t fmt)
{
	es_data_frame *dst_frame = NULL;
	es_error_t ret = ES_SUCCESS;
	
	if((NULL == base)
	|| (NULL == src_frame))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return NULL;
	}
	if(ES_PIXEL_FRAME != src_frame->type)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] pixel convert can not convert other frame!\n" , __FUNCTION__);
        return NULL;
	}
	dst_frame = es_data_frame_create();
	if(NULL == dst_frame)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] es_data_frame_create no mem to create!\n" , __FUNCTION__);
        return NULL;
	}
	dst_frame->type = src_frame->type;
	dst_frame->frame_attr.pix_frame.pix_fmt = fmt;
	ret = do_pix_fmt_convert(src_frame->frame_attr.pix_frame.pix_fmt, src_frame, 
							fmt, dst_frame);
	if(ES_SUCCESS != ret)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] can not support this format convert!\n" , __FUNCTION__);
		es_data_frame_destroy(dst_frame);
		return NULL;
	}
	
	return dst_frame;
}


/*******************************************************************************
* @function name: do_pix_fmt_convert    
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
static es_error_t do_pix_fmt_convert(es_pix_fmt es_src_fmt, struct es_data_frame *src_frame, 
		es_pix_fmt es_dst_fmt, struct es_data_frame *dst_frame)
{
	es_error_t ret = ES_SUCCESS;
	int i ,err; 
	struct es_pix_fmt_info fmt_info;
	uint8_t *src_data[SWSCALE_PLANES_NR];  
	int src_linesize[SWSCALE_PLANES_NR];  
	uint8_t *dst_data[SWSCALE_PLANES_NR];  
	int dst_linesize[SWSCALE_PLANES_NR];    
    int rescale_method = SWS_BILINEAR;  
    struct SwsContext *convert_ctx = NULL; 
    enum AVPixelFormat src_av_pixfmt = es_pix_fmt_to_ffmpeg_fmt(es_src_fmt); 
	enum AVPixelFormat dst_av_pixfmt = es_pix_fmt_to_ffmpeg_fmt(es_dst_fmt); 
	int src_bpp, dst_bpp = 0; 
	const int src_w = src_frame->frame_attr.pix_frame.x_resolution;
	const int src_h = src_frame->frame_attr.pix_frame.y_resolution; 
    const int dst_w = src_frame->frame_attr.pix_frame.x_resolution;
	const int dst_h = src_frame->frame_attr.pix_frame.y_resolution;

	src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_av_pixfmt)); 
	dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_av_pixfmt));

	
    convert_ctx = sws_getContext(src_w, src_h, src_av_pixfmt, 
								dst_w, dst_h, dst_av_pixfmt, 
								SWS_BICUBIC, NULL, NULL, NULL);
	if(NULL == convert_ctx)
   	{
   		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		ret = ES_FAIL;
		goto end;
   	}
	
	err = av_image_alloc(src_data, src_linesize, src_w, src_h, src_av_pixfmt, 1); 
   	if (err < 0) 
   	{  
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Could not allocate source image!\n" , __FUNCTION__); 
       	ret = ES_FAIL;
   	}  
	
   	err = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_av_pixfmt, 1);  
	if (err < 0) 
 	{  
       	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Could not allocate source image!\n" , __FUNCTION__); 
       	ret = ES_FAIL;  
    }

	es_get_pix_fmt_info(es_src_fmt, &fmt_info);
	switch(fmt_info.store_fmt)
	{
		case ES_PIX_STORE_FMT_PACKED:
			memcpy(src_data[0], src_frame->buf_start_addr, (src_bpp >> 3) * src_w * src_h);  
			break;
					
		case ES_PIX_STORE_FMT_PLANAR:	
			for(i = 0; i < SWSCALE_PLANES_NR; i++)
			{
				if((NULL != src_frame->planes[i])
				&& (NULL != src_data[i]))
				{
					memcpy(src_data[i], src_frame->planes[i], src_linesize[i] * src_h);	
				}
			}
			break;
					
		case ES_PIX_STORE_FMT_SEMI_PLANAR:
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Not Support Input Pixel Format!\n" , __FUNCTION__); 
			goto end; 
			break;
			
		default:
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Not Support Input Pixel Format!\n" , __FUNCTION__); 
			goto end; 
			break;
	}

	sws_scale(convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);
	dst_frame->frame_attr.pix_frame.pix_fmt = es_dst_fmt;
	dst_frame->frame_attr.pix_frame.x_resolution = dst_w;
	dst_frame->frame_attr.pix_frame.y_resolution = dst_h;
	es_get_pix_fmt_info(es_dst_fmt, &dst_frame->frame_attr.pix_frame.pix_fmt_info);
	ret = es_data_frame_buf_alloc(dst_frame, src_frame->mem_method, (dst_bpp >> 3) * dst_w * dst_h);
	if(ES_SUCCESS != ret)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] es_data_frame_buf_alloc fail\n" , __FUNCTION__); 
       	ret = ES_FAIL;
		goto end; 	
	}
	
	for(i = 0; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
	{
		dst_frame->planes[i] = NULL;
		dst_frame->plane_bytes[i] = 0;
	}
	
	es_get_pix_fmt_info(es_dst_fmt, &fmt_info);
	switch(fmt_info.store_fmt)
	{
		case ES_PIX_STORE_FMT_PACKED:
			memcpy(dst_frame->buf_start_addr, dst_data[0], (dst_bpp >> 3) * dst_w * dst_h); 
			
			break;
					
		case ES_PIX_STORE_FMT_PLANAR:
			dst_frame->planes[0] = dst_frame->buf_start_addr;
			dst_frame->plane_bytes[0] = dst_linesize[0] * dst_h;
			memcpy(dst_frame->planes[0], dst_data[0], dst_frame->plane_bytes[0]);
				
			for(i = 1; i < SWSCALE_PLANES_NR; i++)
			{
				if(0 == dst_linesize[i])
					break;
						
				dst_frame->plane_bytes[i] = dst_linesize[i] * dst_h;
				dst_frame->planes[i] = dst_frame->planes[i-1] 
									+ dst_frame->plane_bytes[i-1];
				memcpy(dst_frame->planes[i], dst_data[i], dst_frame->plane_bytes[i]);	
			}
			break;
					
		case ES_PIX_STORE_FMT_SEMI_PLANAR:
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Not Support Input Pixel Format!\n" , __FUNCTION__); 
			goto end; 
			break;
			
		default:
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Not Support Input Pixel Format!\n" , __FUNCTION__); 
			goto end; 
			break;
	}
end:
	sws_freeContext(convert_ctx);
	av_freep(&src_data[0]);
	av_freep(&dst_data[0]);
	return ret;


}


static struct frame_convert_base pixel_convert = {
	.type = ES_PIXEL_FRAME,
	.priv = NULL,
	.frame_convert_open = pix_frame_convert_open,
	.frame_convert_close = pix_frame_convert_close,
	.convert_to_spec_frame_fmt = convert_to_spec_pix_frame_fmt,
}; 


es_error_t es_init_pixel_frame_convert()
{
	es_error_t ret = ES_SUCCESS;
	
	ret = frame_convert_base_register(&pixel_convert);
	return ret;

}



