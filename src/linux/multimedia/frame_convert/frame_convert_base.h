/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: frame_convert_base.h 
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
#ifndef _FRAME_CONVERT_BASE_H_
#define _FRAME_CONVERT_BASE_H_
#include <es_frame_convert.h>


typedef struct frame_convert_base{
	es_frame_type type;
	struct es_list_head entry;
	void *priv;

	es_error_t (*frame_convert_open)(struct frame_convert_base *base);
	es_error_t (*frame_convert_close)(struct frame_convert_base *base);
	struct es_media_frame * (*convert_to_spec_frame_fmt)(struct frame_convert_base *base,
		struct es_media_frame *src_frame, frame_fmt_t fmt);
} frame_convert_base;

typedef es_error_t (*sub_init)(void);

extern es_error_t frame_convert_base_register(struct frame_convert_base *pfc);


static inline struct frame_convert_base *frame_convert_base_create()
{
	struct frame_convert_base *pfc = NULL;

	pfc = (struct frame_convert_base *) malloc(sizeof(struct frame_convert_base));
	if(NULL == pfc)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to create frame_convert_base!\n" ,__FUNCTION__);
        return NULL;
	}
	memset(pfc, 0x00, sizeof(struct frame_convert_base));
	return pfc;

}

static inline void frame_convert_base_destroy(struct frame_convert_base * pfc)
{
	if(NULL == pfc)
	{
        return;
	}
	if(NULL != pfc->priv)
	{
		free(pfc->priv);
		pfc->priv = NULL;
	}
	free(pfc);
	pfc = NULL;
}




#endif /* ifndef _FRAME_CONVERT_BASE_H_.2016-12-29 23:57:04 zcz */

