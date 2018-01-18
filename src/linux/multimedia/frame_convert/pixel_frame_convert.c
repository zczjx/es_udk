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


typedef struct pframe_convert_priv_attr{
	unsigned char dummy;
} pframe_convert_priv_attr;


typedef es_error_t (*pix_convert_func)(struct es_data_frame *src_frame, struct es_data_frame *dst_frame);

/*convert from i format to j format*/
static pix_convert_func convert_func_arr[ES_PIX_FMT_NR][ES_PIX_FMT_NR];

/* pixel format convert functions*/
static es_error_t common_rgb_to_yuyv(struct es_data_frame *src_frame, struct es_data_frame *dst_frame);

static es_error_t common_yuyv_to_rgb(struct es_data_frame *src_frame, struct es_data_frame *dst_frame);

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
	int i = 0;
	int j = 0;

	if(NULL == base)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	
	for(i = 0; i < ES_PIX_FMT_NR; i++)
	{
		for(j = 0; j < ES_PIX_FMT_NR; j++)
		{
			convert_func_arr[i][j] = (pix_convert_func) NULL;
		}
	}
	/*convert from i format to j format*/

	/* rgb to yuyv func init*/
	convert_func_arr[ES_PIX_FMT_RGB332][ES_PIX_FMT_YUYV] = common_rgb_to_yuyv;
	convert_func_arr[ES_PIX_FMT_RGB565][ES_PIX_FMT_YUYV] = common_rgb_to_yuyv;
	convert_func_arr[ES_PIX_FMT_RGB24][ES_PIX_FMT_YUYV] = common_rgb_to_yuyv;
	convert_func_arr[ES_PIX_FMT_BGRA32][ES_PIX_FMT_YUYV] = common_rgb_to_yuyv;

	/*yuyv to rgb func init*/
	convert_func_arr[ES_PIX_FMT_YUYV][ES_PIX_FMT_RGB332] = common_yuyv_to_rgb;
	convert_func_arr[ES_PIX_FMT_YUYV][ES_PIX_FMT_RGB565] = common_yuyv_to_rgb;
	convert_func_arr[ES_PIX_FMT_YUYV][ES_PIX_FMT_RGB24] = common_yuyv_to_rgb;
	convert_func_arr[ES_PIX_FMT_YUYV][ES_PIX_FMT_BGRA32] = common_yuyv_to_rgb;

	
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
	int i = ES_PIX_FMT_UNKNOW;
	int j = ES_PIX_FMT_UNKNOW;
	pix_convert_func convert_func  = NULL;

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
	i = src_frame->frame_attr.pix_frame.pix_fmt;
	j = dst_frame->frame_attr.pix_frame.pix_fmt;
	/* assert i j here */
	if((i >= ES_PIX_FMT_NR)
	|| (j >= ES_PIX_FMT_NR))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] pix_fmt is not valiable!\n" , __FUNCTION__);
        return NULL;
	}
	convert_func = convert_func_arr[i][j];
	if(NULL == convert_func)
	{
		return NULL;
	}
	ret = convert_func(src_frame, dst_frame);
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
* @function name: common_rgb_to_yuyv    
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
static es_error_t common_rgb_to_yuyv(struct es_data_frame *src_frame, struct es_data_frame *dst_frame)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}
/*******************************************************************************
* @function name: common_yuyv_to_rgb    
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
static es_error_t common_yuyv_to_rgb(struct es_data_frame *src_frame, struct es_data_frame *dst_frame)
{
	es_error_t ret = ES_SUCCESS;
	int i ,err = 0; 
	static int buf_cnt = 0;
    const int src_w = src_frame->frame_attr.pix_frame.x_resolution;
	const int src_h = src_frame->frame_attr.pix_frame.y_resolution; 
    enum AVPixelFormat src_pixfmt = es_pix_fmt_to_ffmpeg_fmt(src_frame->frame_attr.pix_frame.pix_fmt);  
	int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt)); 
	
    const int dst_w = src_w;
	const int dst_h = src_h;  
    enum AVPixelFormat dst_pixfmt = es_pix_fmt_to_ffmpeg_fmt(dst_frame->frame_attr.pix_frame.pix_fmt);  
    int dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));  

	uint8_t *src_data[4];  
	int src_linesize[4];  
	uint8_t *dst_data[4];  
	int dst_linesize[4];    
    int rescale_method = SWS_BICUBIC;  
    struct SwsContext *convert_ctx = NULL;  
    
   convert_ctx = sws_getContext(src_w, src_h, src_pixfmt,
                             	dst_w, dst_h, dst_pixfmt,
                             	rescale_method, NULL, NULL, NULL);
	if(NULL == convert_ctx)
   	{
   		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		ret = ES_FAIL;
		goto end;
   	}
	
	err = av_image_alloc(src_data, src_linesize,src_w, src_h, src_pixfmt, 32); 

   	if (err < 0) 
   	{  
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Could not allocate source image!\n" , __FUNCTION__); 
       	ret = ES_FAIL;
   	}  
   	err = av_image_alloc(dst_data, dst_linesize,dst_w, dst_h, dst_pixfmt, 32);  

	if (err < 0) 
 	{  
       	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Could not allocate source image!\n" , __FUNCTION__); 
       	ret = ES_FAIL;  
    }
	
	if(AV_PIX_FMT_YUYV422 == src_pixfmt)
	{
		int src_buf_bytes = (src_bpp / 8) * src_w * src_h;
		memcpy(src_data[0], src_frame->buf_start_addr, src_buf_bytes);  
	}
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Not Support Input Pixel Format!\n" , __FUNCTION__); 
       	ret = ES_FAIL;
		goto end; 
	}  
	sws_scale(convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize); 

	dst_frame->frame_attr.pix_frame.x_resolution = dst_w;
	dst_frame->frame_attr.pix_frame.y_resolution = dst_h;
	dst_frame->frame_attr.pix_frame.pix_fmt_info.bits_per_pix = dst_bpp;
	dst_frame->frame_attr.pix_frame.pix_fmt_info.store_fmt = ES_PIX_STORE_FMT_PACKED;
	int dst_buf_bytes = (dst_bpp >> 3) * dst_w * dst_h;
	ret = es_data_frame_buf_alloc(dst_frame, src_frame->mem_method, dst_buf_bytes);
	if(ES_SUCCESS != ret)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] es_data_frame_buf_alloc fail\n" , __FUNCTION__); 
       	ret = ES_FAIL;
		goto end; 	
	}
	
	memcpy(dst_frame->buf_start_addr, dst_data[0], dst_buf_bytes); 
	for(i = 0; i < ES_DATA_FRAME_MAX_PLANE_NR; i++)
	{
			dst_frame->planes[i] = NULL;
			dst_frame->plane_bytes[i] = 0;
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





