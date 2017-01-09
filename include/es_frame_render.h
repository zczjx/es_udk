/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_frame_render.h 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2016-11-8    
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
#ifndef _ES_FRAME_RENDER_H_
#define _ES_FRAME_RENDER_H_

#include <es_common.h>
#include <es_media_frame.h>

typedef void * render_hld;



/* render_type */
#define UNKNOW_RENDER_TYPE		0
#define VIDEO_RENDER_TYPE		1	
#define IMAGE_RENDER_TYPE		2

typedef struct render_attr{
	const char *render_name;
	unsigned long render_type;
} render_attr;

extern es_error_t es_render_init();
extern es_error_t es_render_deinit();
extern es_error_t es_render_open(const char *path, render_hld *r_hld, struct render_attr *r_attr);
extern es_error_t es_render_close(render_hld r_hld);
extern es_error_t es_render_get_attr(render_hld r_hld, struct render_attr *r_attr);
extern es_error_t es_render_set_attr(render_hld r_hld, struct render_attr *r_attr);

/*video and image frame render function*/
extern es_error_t es_frame_full_set_color(render_hld r_hld, unsigned long rgb_val, struct media_frame *frame);

typedef struct es_rect{
	int x_left;
	int x_right;
	int y_top;
	int y_bottom;
} es_rect;

extern es_error_t es_frame_rect_set_color(render_hld r_hld, unsigned long rgb_val, struct es_rect *rect, struct media_frame *frame);
extern es_error_t es_frame_zomm_in(render_hld r_hld, unsigned long scale, struct media_frame *frame);
extern es_error_t es_frame_zoom_out(render_hld r_hld, unsigned long scale, struct media_frame *frame);

/*clip src_f --> a rectangle region of dst_f*/
extern es_error_t es_frame_clip(render_hld r_hld, struct es_rect *rect, 
								struct media_frame *src_f, struct media_frame *dst_f);







#endif /* ifndef _ES_FRAME_RENDER_H_.2016-11-8 22:12:10 zcz */


