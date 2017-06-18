/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_frame_convert.h 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2016-12-29    
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
#ifndef _ES_FRAME_CONVERT_H_
#define _ES_FRAME_CONVERT_H_
#include <es_list.h>
#include <es_common.h>
#include <es_data_frame.h>

typedef void * convert_hld;



extern es_error_t es_frame_convert_init();

extern es_error_t es_frame_convert_deinit();

extern es_error_t es_frame_convert_open(es_frame_type type, convert_hld *c_hld);

extern es_error_t es_frame_convert_close(convert_hld c_hld);

extern es_frame_type es_get_convert_frame_type(convert_hld c_hld);

extern struct es_data_frame * es_convert_to_spec_frame_fmt(convert_hld c_hld, struct es_data_frame *src_frame,
															es_multimedia_fmt_t fmt);

#endif /* ifndef _ES_FRAME_CONVERT_H_.2016-12-29 22:27:48 zcz */





