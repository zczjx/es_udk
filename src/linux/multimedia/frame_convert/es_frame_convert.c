/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_frame_convert.c 
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
#include <es_frame_convert.h>
	
#include "frame_convert_base.h"
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ES_LIST_HEAD(frame_convert_head);

extern es_error_t es_init_pixel_frame_convert();

static sub_init es_frame_convert_init_arr[] = {
	es_init_pixel_frame_convert,
	NULL,
};

/*******************************************************************************
* @function name: do_frame_convert_base_instantiate    
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
static void inline do_frame_convert_base_instantiate(struct frame_convert_base *instance, 
	struct frame_convert_base *der_class)
{
	instance->type = der_class->type;
	INIT_ES_LIST_HEAD(&instance->entry);
	instance->priv = NULL;
	instance->frame_convert_open = der_class->frame_convert_open;
	instance->frame_convert_close = der_class->frame_convert_close;
	instance->convert_to_spec_frame_fmt = der_class->convert_to_spec_frame_fmt;
}



/*******************************************************************************
* @function name: frame_convert_base_register    
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
es_error_t frame_convert_base_register(struct frame_convert_base *pfc)
{
	if(NULL == pfc)
		return ES_INVALID_PARAM;
	
	es_list_add(&pfc->entry, &frame_convert_head);
	return ES_SUCCESS;

}


/*******************************************************************************
* @function name: es_frame_convert_init    
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
es_error_t es_frame_convert_init()
{
	int i = 0;
	es_error_t ret = ES_SUCCESS;
	sub_init init_func = NULL;
	int arr_len = (sizeof(es_frame_convert_init_arr) / sizeof(long));

	for(i = 0; i < arr_len; i++)
	{
		if(NULL != es_frame_convert_init_arr[i])
		{
			init_func = es_frame_convert_init_arr[i];
			ret = init_func();
		}
	}
	return ret;

}

/*******************************************************************************
* @function name: es_frame_convert_deinit    
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
es_error_t es_frame_convert_deinit()
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}
/*******************************************************************************
* @function name: es_frame_convert_open    
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
es_error_t es_frame_convert_open(es_frame_type type, convert_hld *c_hld)
{
	es_error_t ret = ES_SUCCESS;
	convert_hld es_hld = NULL;
	struct frame_convert_base *pos_base = NULL;
	struct frame_convert_base *instance = NULL;
	int i = 0;
	
	if(NULL == c_hld)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	es_list_for_each_entry(pos_base, &frame_convert_head, entry)
	{
		if(type == pos_base->type)
		{
			instance = frame_convert_base_create();
			if(NULL != instance)
			{
				do_frame_convert_base_instantiate(instance, pos_base);
				ret = instance->frame_convert_open(instance);
				if(ES_SUCCESS == ret)
				{	
					es_hld = (convert_hld) instance;
					*c_hld = es_hld;
					return ret;
				}
				else
				{
					frame_convert_base_destroy(instance);
					es_hld = NULL;
					*c_hld = es_hld;
					return ret;
				}
			}
			else
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] no mem to alloc!\n" , __FUNCTION__);
				es_hld = NULL;
				*c_hld = es_hld;
				return ES_FAIL;
			}
		}
	}
	ret = ES_FAIL;
	es_hld = NULL;
	*c_hld = es_hld;
	return ret;
}

/*******************************************************************************
* @function name: es_frame_convert_close    
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
es_error_t es_frame_convert_close(convert_hld c_hld)
{
	struct frame_convert_base *pfcb = (struct display_base *) c_hld;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pfcb)
	{
		return ES_INVALID_PARAM;
	}
	
	if(NULL != pfcb->frame_convert_close)
	{
		ret = pfcb->frame_convert_close(pfcb);
		frame_convert_base_destroy(pfcb);
	}
	else
	{
		return ES_INVALID_PARAM;
	}
		
	return ret;
}


/*******************************************************************************
* @function name: es_get_convert_frame_type    
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
es_frame_type es_get_convert_frame_type(convert_hld c_hld)
{
	struct frame_convert_base *pfcb = (struct display_base *) c_hld;
	es_frame_type ret = ES_UNKNOW_FRAME;

	if(NULL == pfcb)
	{
		return ES_UNKNOW_FRAME;
	}
	return pfcb->type;
}

/*******************************************************************************
* @function name: es_convert_to_spec_frame_fmt    
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
struct es_data_frame * es_convert_to_spec_frame_fmt(convert_hld c_hld, struct es_data_frame *src_frame,
	es_multimedia_fmt_t fmt)
{
	es_error_t ret = ES_SUCCESS;
	struct frame_convert_base *pfcb = (struct display_base *) c_hld;

	if((NULL == c_hld) 
	|| (NULL == src_frame))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return NULL;
	}
	if(es_get_convert_frame_type(c_hld) != src_frame->type)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] convert type is not compatible!\n" , __FUNCTION__);
		return NULL;
	}
	
	if(NULL == pfcb->convert_to_spec_frame_fmt)
	{
		return NULL;
	}
	return pfcb->convert_to_spec_frame_fmt(pfcb, src_frame, fmt);
}
															



