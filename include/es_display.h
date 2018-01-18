/*******************************************************************************
* Copyright (C), 2000-2018,  Electronic Technology Co., Ltd.
*                
* @filename: es_display.h 
*                
* @author: Clarence.Zhou <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2018-1-12    
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
#ifndef _ES_DISPLAY_H_
#define _ES_DISPLAY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <es_common.h>
#include <es_list.h>
#include <es_data_frame.h>

typedef void * es_disp_hld;

typedef enum {
	ES_DISPLAY_CLASS_UNKNOW = 0,
	ES_DISPLAY_CLASS_FB,
	ES_DISPLAY_CLASS_CRT,
} es_display_class;


typedef struct es_display_attr {
	es_pix_fmt			pix_fmt; //ro
	struct es_pix_fmt_info pix_fmt_info;
	unsigned long 		x_resolution; //ro
	unsigned long 		y_resolution; //ro
} es_display_attr;

extern es_error_t es_display_init();
extern es_error_t es_display_deinit();
extern es_error_t es_display_open(const char *path, es_display_class sub_class, es_disp_hld *d_hld);
extern es_error_t es_display_close(es_disp_hld d_hld);
extern es_error_t es_display_get_attr(es_disp_hld d_hld, struct es_display_attr *d_attr);
extern es_error_t es_display_set_attr(es_disp_hld d_hld, struct es_display_attr *d_attr);
extern es_error_t es_display_on(es_disp_hld d_hld);
extern es_error_t es_display_off(es_disp_hld d_hld);

typedef (*disp_callback)(struct es_data_frame *dframe, void *arg);
extern es_error_t es_display_async_flush(es_disp_hld d_hld, struct es_data_frame *dframe,
										disp_callback dcb, void *arg);
extern es_error_t es_display_sync_flush(es_disp_hld d_hld, struct es_data_frame *dframe);

extern es_error_t es_display_async_grab(es_disp_hld d_hld, disp_callback dcb, void *arg);
extern es_error_t es_display_sync_grab(es_disp_hld d_hld, struct es_data_frame *dframe);


#ifdef __cplusplus
}
#endif

#endif /* ifndef _ES_DISPLAY_H_.2018-1-12 22:26:58 zcz */


