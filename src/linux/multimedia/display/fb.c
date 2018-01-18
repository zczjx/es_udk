/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: fb.c 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2016-12-4    
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
#include <es_display.h>
#include <es_data_frame.h>
#include "display_base.h"

#include <linux/fb.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

typedef struct fb_priv_attr{
	__u32 bytes_per_line;
	__u32 fb_size;
	unsigned char *fbmem;
} fb_priv_attr;

/*******************************************************************/
#define BYTES_PER_LINE(var) (var.xres * var.bits_per_pixel / 8)	
#define BYTES_PER_PIXEL(var) (var.bits_per_pixel / 8)	
#define BYTES_FB_SIZE(var) (var.xres * var.yres * var.bits_per_pixel / 8)	
#define CLR_LCD_FB(fbmem,size,color) do{memset(fbmem, color, size);}while(0)

/***********************************************************/
static inline es_pix_fmt get_fb_pix_fmt(struct fb_var_screeninfo *fb_var_info)
{
	es_pix_fmt ret = ES_PIX_FMT_UNKNOW;
	
	if(NULL == fb_var_info)
	{
		return ES_PIX_FMT_UNKNOW;	
	}
	switch(fb_var_info->bits_per_pixel)
	{
		case 8:
				ret = ES_PIX_FMT_RGB332;
				break;

		case 16:
				ret = ES_PIX_FMT_RGB565;
				break;
		case 24:
				ret = ES_PIX_FMT_RGB24;
				break;

		case 32:
				ret = ES_PIX_FMT_BGRA32;
				break;
		default:
				ret = ES_PIX_FMT_UNKNOW;
				break;		
	}
	return ret;
}


/*******************************************************************************
* @function name: do_flush_rgb_frame    
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
static inline es_error_t do_flush_rgb_frame(struct fb_priv_attr *priv_attr, 
	struct es_display_attr *public_attr, 
	struct es_data_frame *dframe)
	
{
	unsigned long flush_line_bytes;
	unsigned long flush_line_nr;

	unsigned long frame_line_bytes;
	unsigned long fb_line_bytes;

	unsigned char *frame_src_addr;
	unsigned char *fb_dst_addr;
	int i = 0;
	
	
	es_error_t ret = ES_SUCCESS;
	struct es_pixel_frame_attr *pix_frame_attr = &dframe->frame_attr.pix_frame;

	if((NULL == priv_attr)
	|| (NULL == public_attr)
	|| (NULL == pix_frame_attr)
	|| (NULL == dframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

	
	if((pix_frame_attr->pix_fmt_info.bits_per_pix != public_attr->pix_fmt_info.bits_per_pix)
	|| (pix_frame_attr->pix_fmt_info.store_fmt != public_attr->pix_fmt_info.store_fmt))
	{	
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] frame's bpp is not compatible with display screen!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	frame_line_bytes = pix_frame_attr->x_resolution * (pix_frame_attr->pix_fmt_info.bits_per_pix >> 3);
 	fb_line_bytes = public_attr->x_resolution * (public_attr->pix_fmt_info.bits_per_pix >> 3);
	flush_line_bytes = min(frame_line_bytes, fb_line_bytes);
	flush_line_nr = min(pix_frame_attr->y_resolution, public_attr->y_resolution);
	fb_dst_addr = priv_attr->fbmem;
	frame_src_addr = dframe->buf_start_addr;
	for(i = 0; i < flush_line_nr; i++)
	{
		memcpy(fb_dst_addr, frame_src_addr, flush_line_bytes);
		fb_dst_addr += fb_line_bytes;
		frame_src_addr += frame_line_bytes;
	}
	
	return ES_SUCCESS;	
}



/***************************************************************/

static es_error_t fb_open(const char *path, struct display_base *base);
static es_error_t fb_close(struct display_base *base);
static es_error_t fb_on(struct display_base *base);
static es_error_t fb_off(struct display_base *base);
static es_error_t fb_flush(struct display_base *base, struct es_data_frame *dframe);
static es_error_t fb_grab(struct display_base *base, struct es_data_frame **dframe);



static struct display_base fb_dev = {
	.sub_class = ES_DISPLAY_CLASS_FB,
	.fd = -1,
	.priv = NULL,

	.display_open = fb_open,
	.display_close = fb_close,
	.display_on = fb_on,
	.display_off = fb_off,
	.display_flush = fb_flush,
	.display_grab = fb_grab,
};


/*******************************************************************************
* @function name: fb_open    
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
static es_error_t fb_open(const char *path, struct display_base *base)
{
	int i;
    es_error_t ret;
	int err;
	struct fb_priv_attr *priv_attr = NULL;
	struct es_display_attr public_attr;
	int fd;
	struct fb_var_screeninfo fb_var_info;
	struct fb_fix_screeninfo fb_fix_info;

	if((NULL == path) || (NULL == base))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	priv_attr = malloc(sizeof(struct fb_priv_attr));
	if(NULL == priv_attr)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no memory now!\n" , __FUNCTION__);
        return ES_FAIL;
	}
	fd = open(path, O_RDWR);
	if (fd < 0)
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] can not open %s!\n" , __FUNCTION__, path);
        return ES_FAIL;
    }
	err = ioctl(fd, FBIOGET_VSCREENINFO, &fb_var_info);
	if (err)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] can't get var-info of screen!\n" , __FUNCTION__);
		return ES_FAIL;
	}
	err = ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix_info);
	if (err)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] can't get fix-info of screen!\n" , __FUNCTION__);
		return ES_FAIL;
	}
	priv_attr->bytes_per_line = BYTES_PER_LINE(fb_var_info);
	priv_attr->fb_size = BYTES_FB_SIZE(fb_var_info);
	priv_attr->fbmem = (unsigned char *)mmap(NULL, priv_attr->fb_size,
										(PROT_READ | PROT_WRITE), MAP_SHARED,
										fd, 0);
	
	public_attr.x_resolution = fb_var_info.xres;
	public_attr.y_resolution = fb_var_info.yres;
	public_attr.pix_fmt = get_fb_pix_fmt(&fb_var_info);
	es_get_pix_fmt_info(public_attr.pix_fmt , &public_attr.pix_fmt_info);

	base->sub_class = ES_DISPLAY_CLASS_FB;
	base->path_name = path;
	base->fd = fd;
	memcpy(&base->attr, &public_attr, sizeof(struct es_display_attr));
	INIT_ES_LIST_HEAD(&base->entry);
	base->priv = priv_attr;
    return ES_SUCCESS;
}
/*******************************************************************************
* @function name: fb_close    
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
static es_error_t fb_close(struct display_base *base)
{
	struct fb_priv_attr  *priv_attr = base->priv;
	int i = 0;
	int err;
	es_error_t ret = ES_SUCCESS;

	if((NULL == base) || (NULL == priv_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	
	err = munmap(priv_attr->fbmem, priv_attr->fb_size);
	if(err)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] munmap fail in close!\n" , __FUNCTION__);
	}
			
	free(priv_attr);
	priv_attr = NULL;
	ret = close(base->fd);
	return ret;
}
/*******************************************************************************
* @function name: fb_on    
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
static es_error_t fb_on(struct display_base *base)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}
/*******************************************************************************
* @function name: fb_off    
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
static es_error_t fb_off(struct display_base *base)
{
	es_error_t ret = ES_SUCCESS;

	return ret;
}
/*******************************************************************************
* @function name: fb_flush    
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
static es_error_t fb_flush(struct display_base *base, struct es_data_frame *dframe)
{
	es_error_t ret = ES_SUCCESS;
	struct fb_priv_attr *priv_attr = base->priv;
	struct es_display_attr *public_attr = &base->attr;

	if((NULL == base) 
		|| (NULL == priv_attr)
		|| (NULL == public_attr)
		|| (NULL == dframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(public_attr->pix_fmt == dframe->frame_attr.pix_frame.pix_fmt)
	{
		ret = do_flush_rgb_frame(priv_attr, public_attr, dframe);
	}
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] device and frame 's pixel format is not match!\n" , __FUNCTION__);
		ret = ES_INVALID_PARAM;
	}

	return ret;
}
/*******************************************************************************
* @function name: fb_grab    
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
static es_error_t fb_grab(struct display_base *base, struct es_data_frame **dframe)
{
	es_error_t ret = ES_SUCCESS;

	return ret;
}


/*******************************************************************************
* @function name: es_init_fb    
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
es_error_t es_init_fb(void)
{
	es_error_t ret;
	
	ret = display_base_register(&fb_dev);
	return ret;
}

