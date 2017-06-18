/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_display.c 
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

#include "display_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ES_LIST_HEAD(display_base_head);

extern es_error_t es_init_fb();

static sub_init es_display_init_arr[] = {
//	es_init_demo_display,
	es_init_fb,
	NULL,
};



/*******************************************************************************
* @function name: do_display_base_instanceiate    
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
static void inline do_display_base_instantiate(struct display_base *instance, 
	struct display_base *der_class)
{
	instance->attr.pix_fmt = ES_PIX_FMT_UNKNOW;
	instance->attr.x_resolution = 0;
	instance->attr.y_resolution = 0;
	instance->attr.bits_per_pix = 0;
	INIT_ES_LIST_HEAD(&instance->entry);
	instance->priv = NULL;
	instance->sub_class = der_class->sub_class;
	instance->display_open = der_class->display_open;
	instance->display_close = der_class->display_close;
	instance->display_on = der_class->display_on;
	instance->display_off = der_class->display_off;
	instance->display_flush = der_class->display_flush;
	instance->display_grab = der_class->display_grab;
}

/*******************************************************************************
* @function name: display_base_register    
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
es_error_t display_base_register(struct display_base *pd)
{
	if(NULL == pd)
		return ES_INVALID_PARAM;
	
	es_list_add(&pd->entry, &display_base_head);
	return ES_SUCCESS;	

}


/*******************************************************************************
* @function name: es_display_init    
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
es_error_t es_display_init()
{
	int i = 0;
	es_error_t ret = ES_SUCCESS;
	sub_init init_func = NULL;
	int arr_len = (sizeof(es_display_init_arr) / sizeof(long));

	for(i = 0; i < arr_len; i++)
	{
		if(NULL != es_display_init_arr[i])
		{
			init_func = es_display_init_arr[i];
			ret = init_func();
		}
	}
	return ret;
}
/*******************************************************************************
* @function name: es_display_deinit    
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
es_error_t es_display_deinit()
{
	es_error_t ret = ES_SUCCESS;

	return ret;
}
/*******************************************************************************
* @function name: es_display_open    
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
es_error_t es_display_open(const char *path, es_display_class sub_class, es_disp_hld *d_hld)
{
	es_error_t ret = ES_SUCCESS;
	es_disp_hld es_hld = NULL;
	struct display_base *pos_base = NULL;
	struct display_base *instance = NULL;
	int i = 0;
	
	if((NULL == path) || (NULL == d_hld))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	
	if(0 == strcmp(path, ES_DEFAULT_CONFIG_PATH))
	{
		es_hld = NULL;
		*d_hld = es_hld;
		return ret;
	}
	else if(0 == strcmp(path, ES_DEFAULT_INTERNAL_DEV))
	{
		es_hld = NULL;
		*d_hld = es_hld;
		return ret;
	}
	else
	{
		es_list_for_each_entry(pos_base, &display_base_head, entry)
		{
			if(sub_class == pos_base->sub_class)
			{
				instance = display_base_create();
				if(NULL != instance)
				{
					do_display_base_instantiate(instance, pos_base);
					ret = instance->display_open(path, instance);
					if(ES_SUCCESS == ret)
					{	
						es_hld = (es_disp_hld) instance;
						*d_hld = es_hld;
						return ret;
					}
					else
					{
						display_base_destroy(instance);
						es_hld = NULL;
						*d_hld = es_hld;
						return ret;
					}
				}
				else
				{
					ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
					ES_PRINTF("[%s] no mem to alloc!\n" , __FUNCTION__);
					es_hld = NULL;
					*d_hld = es_hld;
					return ES_FAIL;
				}
			}
		}
		ret = ES_FAIL;
		es_hld = NULL;
		*d_hld = es_hld;
		return ret;
	}
}
/*******************************************************************************
* @function name: es_display_close    
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
es_error_t es_display_close(es_disp_hld d_hld)
{
	struct display_base *pdb = (struct display_base *) d_hld;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pdb)
		return ES_INVALID_PARAM;
	if(NULL != pdb->display_close)
	{
		ret = pdb->display_close(pdb);
		display_base_destroy(pdb);
	}
	else
		return ES_INVALID_PARAM;
	return ret;
}
/*******************************************************************************
* @function name: es_display_get_attr    
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
es_error_t es_display_get_attr(es_disp_hld d_hld, struct es_display_attr *d_attr)
{
	es_error_t ret = ES_SUCCESS;
	struct display_base *pdb = (struct display_base *) d_hld;

	if((NULL == pdb) || (NULL == d_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	memcpy(d_attr, &pdb->attr, sizeof(struct es_display_attr));
	ret = ES_SUCCESS;
	return ret;

}
/*******************************************************************************
* @function name: es_display_set_attr    
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
es_error_t es_display_set_attr(es_disp_hld d_hld, struct es_display_attr *d_attr)
{
	es_error_t ret = ES_SUCCESS;
	struct display_base *pdb = (struct display_base *) d_hld;

	if((NULL == pdb) || (NULL == d_attr))
		return ES_INVALID_PARAM;
	
	memcpy(&pdb->attr, d_attr, sizeof(struct es_display_attr));
	if((NULL != pdb->display_on) && (NULL != pdb->display_off))
	{	
		ret = pdb->display_off(pdb);
		if(ES_SUCCESS != ret)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] display close fail!\n" , __FUNCTION__);
			return ret;
		}
		es_common_delay(100);
		ret = pdb->display_on(pdb);
		if(ES_SUCCESS != ret)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] display restart fail!\n" , __FUNCTION__);
			return ret;
		}
	}
	return ret;
}

/*******************************************************************************
* @function name: es_display_on    
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
extern es_error_t es_display_on(es_disp_hld d_hld)
{
	struct display_base *pdb = (struct display_base *) d_hld;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pdb)
		return ES_INVALID_PARAM;
	if(NULL != pdb->display_on)
		ret = pdb->display_on(pdb);
	else
		return ES_INVALID_PARAM;
	return ret;
}
/*******************************************************************************
* @function name: es_display_off    
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
extern es_error_t es_display_off(es_disp_hld d_hld)
{
	struct display_base *pdb = (struct display_base *) d_hld;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pdb)
		return ES_INVALID_PARAM;
	if(NULL != pdb->display_off)
		ret = pdb->display_off(pdb);
	else
		return ES_INVALID_PARAM;
	return ret;
}


/*******************************************************************************
* @function name: es_display_async_flush    
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
es_error_t es_display_async_flush(es_disp_hld d_hld, struct es_data_frame *dframe,
								disp_callback dcb, void *arg)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}
/*******************************************************************************
* @function name: es_display_sync_flush    
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
es_error_t es_display_sync_flush(es_disp_hld d_hld, struct es_data_frame *dframe)
{
	es_error_t ret = ES_SUCCESS;
	struct display_base *pdb = (struct display_base *) d_hld;

	if((NULL == pdb) || (NULL == dframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(ES_PIXEL_FRAME != dframe->type)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no pixel frame to flush display!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	
	if(NULL != pdb->display_flush)
	{
		ret = pdb->display_flush(pdb, dframe);
	}
	else
	{
		return ES_INVALID_PARAM;
	}
	return ret;
}

/*******************************************************************************
* @function name: es_display_async_grab    
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
es_error_t es_display_async_grab(es_disp_hld d_hld, disp_callback dcb, void *arg)
{	
	es_error_t ret = ES_SUCCESS;

	return ret;
}
/*******************************************************************************
* @function name: es_display_sync_grab    
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
es_error_t es_display_sync_grab(es_disp_hld d_hld, struct es_data_frame *dframe)
{
	es_error_t ret = ES_SUCCESS;
	struct display_base *pdb = (struct display_base *) d_hld;

	if((NULL == pdb) || (NULL == dframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pdb->display_grab)
	{
		ret = pdb->display_grab(pdb, dframe);
	}
	else
	{
		return ES_INVALID_PARAM;
	}
	return ret;
}


