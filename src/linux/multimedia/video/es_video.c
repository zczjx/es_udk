/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_video.c 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2016-11-11    
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
#define ES_DEBUG 1
#include <es_common.h>
#include <es_video.h>

#include "video_base.h"

#include <stdlib.h>
#include <string.h>


ES_LIST_HEAD(video_base_head);

// extern es_error_t es_init_demo_video();
extern es_error_t es_init_v4l2_camera();

static sub_init es_video_init_arr[] = {
//	es_init_demo_video,
	es_init_v4l2_camera,
	NULL,
};


/*******************************************************************************
* @function name: do_video_base_instantiate    
*                
* @brief:          
*                
* @param:        
*	@instance[in]
*	@der_class[in]
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
static void inline do_video_base_instantiate(struct video_base *instance, 
	struct video_base *der_class)
{
	instance->attr.property = ES_VIDEO_PROPERTY_UNKNOW;
	INIT_ES_LIST_HEAD(&instance->ctrl_list_head);
	INIT_ES_LIST_HEAD(&instance->entry);
	INIT_ES_LIST_HEAD(&instance->video_buf_head);
	instance->priv = NULL;
	instance->sub_class = der_class->sub_class;
	instance->video_open = der_class->video_open;
	instance->video_close = der_class->video_close;
	instance->video_start = der_class->video_start;
	instance->video_stop = der_class->video_stop;
	instance->video_get_ctrl = der_class->video_get_ctrl;
	instance->video_set_ctrl = der_class->video_set_ctrl;
	instance->video_send_frame = der_class->video_send_frame;
	instance->video_recv_frame = der_class->video_recv_frame;
}

/*******************************************************************************
* @function name: register_video_base    
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
es_error_t video_base_register(struct video_base *pv)
{
	if(NULL == pv)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	
	es_list_add(&pv->entry, &video_base_head);
	return ES_SUCCESS;	
}


/*******************************************************************************
* @function name: es_video_init    
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
es_error_t es_video_init()
{
	int i = 0;
	es_error_t ret = ES_SUCCESS;
	sub_init init_func = NULL;
	int arr_len = (sizeof(es_video_init_arr) / sizeof(long));

	for(i = 0; i < arr_len; i++)
	{
		if(NULL != es_video_init_arr[i])
		{
			init_func = es_video_init_arr[i];
			ret = init_func();
		}
	}
	return ret;
}
/*******************************************************************************
* @function name: es_video_deinit    
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
es_error_t es_video_deinit()
{
	es_error_t ret = ES_SUCCESS;

	return ret;
}

/*******************************************************************************
* @function name: es_video_open    
*                
* @brief:          
*                
* @param: 
* 	@path[in]
*	@sub_class[in]
*	@v_hld[out]
*                
* @return:        
*                
* @comment:        
*******************************************************************************/
es_error_t es_video_open(const char *path,
	es_video_class sub_class,
	es_video_hld *v_hld)
{
	es_error_t ret = ES_SUCCESS;
	es_video_hld es_hld = NULL;
	struct video_base *pos_base = NULL;
	struct video_base *instance = NULL;
	int i = 0;
	
	if((NULL == path) || (NULL == v_hld))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	
	if(0 == strcmp(path, ES_DEFAULT_CONFIG_PATH))
	{
		es_hld = NULL;
		*v_hld = es_hld;
		return ret;
	}
	else if(0 == strcmp(path, ES_DEFAULT_INTERNAL_DEV))
	{
		es_hld = NULL;
		*v_hld = es_hld;
		return ret;
	}
	else
	{
		es_list_for_each_entry(pos_base, &video_base_head, entry)
		{
			if(sub_class == pos_base->sub_class)
			{
				instance = video_base_create();
				if(NULL != instance)
				{
					do_video_base_instantiate(instance, pos_base);
					ret = instance->video_open(path, instance);
					if(ES_SUCCESS == ret)
					{	
						es_hld = (es_video_hld) instance;
						*v_hld = es_hld;
						return ret;
					}
					else
					{
						video_base_destroy(instance);
						es_hld = NULL;
						*v_hld = es_hld;
						return ret;
					}
				}
				else
				{
					ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
					ES_PRINTF("[%s] no enough mem to alloc inistant!\n" , __FUNCTION__);
					es_hld = NULL;
					*v_hld = es_hld;
					return ES_FAIL;
				}
			}
		}
		ret = ES_FAIL;
		es_hld = NULL;
		*v_hld = es_hld;
		return ret;
	}
}
/*******************************************************************************
* @function name: es_video_close    
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
es_error_t es_video_close(es_video_hld v_hld)
{
	struct video_base *pvb = (struct video_base *) v_hld;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pvb)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_close)
	{
		ret = pvb->video_close(pvb);
		video_base_destroy(pvb);
	}
	else
	{
		ret =  ES_INVALID_PARAM;
	}
	return ret;
}
/*******************************************************************************
* @function name: es_video_get_attr    
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
es_error_t es_video_get_attr(es_video_hld v_hld, struct es_video_attr *v_attr)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;

	if((NULL == pvb) || (NULL == v_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	memcpy(v_attr, &pvb->attr, sizeof(struct es_video_attr));
	ret = ES_SUCCESS;
	return ret;
}
/*******************************************************************************
* @function name: es_video_set_attr    
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
es_error_t es_video_set_attr(es_video_hld v_hld, struct es_video_attr *v_attr)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;

	if((NULL == pvb) || (NULL == v_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	memcpy(&pvb->attr, v_attr, sizeof(struct es_video_attr));
	if((NULL != pvb->video_start) && (NULL != pvb->video_stop))
	{	
		ret = pvb->video_stop(pvb);
		if(ES_SUCCESS != ret)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] video stop fail!\n" , __FUNCTION__);
			return ret;
		}
		es_common_delay(100);
		ret = pvb->video_start(pvb);
		if(ES_SUCCESS != ret)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] video restart fail!\n" , __FUNCTION__);
			return ret;
		}
	}
	return ret;
}
/*******************************************************************************
* @function name: es_video_start    
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
es_error_t es_video_start(es_video_hld v_hld)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;
	
	if(NULL == pvb)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_start)
	{
		ret = pvb->video_start(pvb);
	}
	else
	{
		ret =  ES_INVALID_PARAM;
	}
	return ret;
	
}
/*******************************************************************************
* @function name: es_video_stop    
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
es_error_t es_video_stop(es_video_hld v_hld)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;
	
	if(NULL == pvb)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_stop)
	{
		ret = pvb->video_stop(pvb);
	}
	else
	{
		ret = ES_INVALID_PARAM;
	}
	return ret;
}

/*******************************************************************************
* @function name: es_video_async_recv_frame    
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
es_error_t es_video_async_recv_frame(es_video_hld v_hld,
	video_callback vcb, 
	void *arg)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}
/*******************************************************************************
* @function name: es_video_sync_recv_frame    
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
es_error_t es_video_sync_recv_frame(es_video_hld v_hld, 
	struct es_media_frame *vframe)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;

	if((NULL == pvb) || (NULL == vframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_recv_frame)
	{
		ret = pvb->video_recv_frame(pvb, vframe);
	}
	else
	{
		ret = ES_INVALID_PARAM;
	}
	return ret;

}
/*******************************************************************************
* @function name: es_video_async_send_frame    
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
es_error_t es_video_async_send_frame(es_video_hld v_hld, 
	struct es_media_frame *vframe, 
	video_callback vcb, 
	void *arg)
{
	es_error_t ret = ES_SUCCESS;

	return ret;

}
/*******************************************************************************
* @function name: es_video_sync_send_frame    
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
es_error_t es_video_sync_send_frame(es_video_hld v_hld, 
	struct es_media_frame *vframe)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;

	if((NULL == pvb) || (NULL == vframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_send_frame)
	{
		ret = pvb->video_send_frame(pvb, vframe);
	}
	else
	{
		ret = ES_INVALID_PARAM;
	}
	return ret;
}

/*******************************************************************************
* @function name: es_video_get_ctrl    
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
es_error_t es_video_get_ctrl(es_video_hld v_hld, struct es_video_ctrl_cmd *cmd)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;

	if((NULL == pvb) || (NULL == cmd))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_get_ctrl)
	{
		ret = pvb->video_get_ctrl(pvb, cmd);
	}
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] ES_INVALID_PARAM!\n" , __FUNCTION__);
		ret = ES_INVALID_PARAM;
	}
	return ret;
	

}

/*******************************************************************************
* @function name: es_video_set_ctrl    
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
es_error_t es_video_set_ctrl(es_video_hld v_hld, struct es_video_ctrl_cmd *cmd)
{
	es_error_t ret = ES_SUCCESS;
	struct video_base *pvb = (struct video_base *) v_hld;

	if((NULL == pvb) || (NULL == cmd))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pvb->video_set_ctrl)
	{
		ret = pvb->video_set_ctrl(pvb, cmd);
	}
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] ES_INVALID_PARAM!\n" , __FUNCTION__);
		ret = ES_INVALID_PARAM;
	}
	return ret;


}


