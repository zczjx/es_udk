/*******************************************************************************
* Copyright (C), 2000-2018,  Electronic Technology Co., Ltd.
*                
* @filename: es_codec.c 
*                
* @author: Clarence.Zhou <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2018-1-9    
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
#include <es_codec.h>	
#include "codec_base.h"
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ES_LIST_HEAD(codec_head);

extern es_error_t es_init_ffmpeg_audio_codec();
extern es_error_t es_init_ffmpeg_image_codec();
extern es_error_t es_init_ffmpeg_video_codec();

static sub_init es_codec_init_arr[] = {
	es_init_ffmpeg_audio_codec,
	es_init_ffmpeg_image_codec,
	es_init_ffmpeg_video_codec,
	NULL,
};

static bool is_codec_support(struct codec_base *base, struct es_codec_attr *c_attr)
{
	if((NULL == base) || (NULL == c_attr))
		return es_false;

	if((ES_CODEC_CLASS_UNKNOW == c_attr->codec_class)
	|| ES_CODEC_CAP_UNKNOW == c_attr->codec_cap)
		return es_false;

	return base->is_codec_support(c_attr->codec_class, c_attr->codec_fmt, c_attr->codec_cap);
}
/*******************************************************************************
* @function name: do_codec_base_instantiate    
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
static void inline do_codec_base_instantiate(struct codec_base *instance, 
	struct codec_base *der_class)
{
	instance->sub_class = der_class->sub_class;
	INIT_ES_LIST_HEAD(&instance->entry);
	instance->attr.codec_class = ES_CODEC_CLASS_UNKNOW;
	memset(&instance->attr, 0, sizeof(instance->attr));
	instance->priv = NULL;
	instance->decode_priv_desc = NULL;
	instance->encode_priv_desc = NULL;
	instance->is_codec_support = der_class->is_codec_support;
	instance->codec_open = der_class->codec_open;
	instance->codec_close = der_class->codec_close;
	instance->codec_decode = der_class->codec_decode;
	instance->codec_encode = der_class->codec_encode;
}



/*******************************************************************************
* @function name: codec_base_register    
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
es_error_t codec_base_register(struct codec_base *pcodec)
{
	if(NULL == pcodec)
		return ES_INVALID_PARAM;
	
	es_list_add(&pcodec->entry, &codec_head);
	return ES_SUCCESS;

}

/*******************************************************************************
* @function name: es_codec_init    
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
es_error_t es_codec_init()
{
	int i = 0;
	es_error_t ret = ES_SUCCESS;
	sub_init init_func = NULL;
	int arr_len = (sizeof(es_codec_init_arr) / sizeof(long));

	for(i = 0; i < arr_len; i++)
	{
		if(NULL != es_codec_init_arr[i])
		{
			init_func = es_codec_init_arr[i];
			ret = init_func();
		}
	}
	return ret;

}

/*******************************************************************************
* @function name: es_codec_deinit    
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
es_error_t es_codec_deinit()
{
	es_error_t ret = ES_SUCCESS;

	return ret;
}

/*******************************************************************************
* @function name: es_codec_open    
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
es_error_t es_codec_open(struct es_codec_attr *c_attr, es_codec_hld *c_hld)
{
	es_error_t ret = ES_SUCCESS;
	es_codec_hld es_hld = NULL;
	struct codec_base *pos_base = NULL;
	struct codec_base *instance = NULL;
	int i = 0;
	
	if((NULL == c_attr) || (NULL == c_hld))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	
	
	es_list_for_each_entry(pos_base, &codec_head, entry)
	{
		if(c_attr->codec_class == pos_base->sub_class)
		{
			if(es_false == is_codec_support(pos_base, c_attr))
				continue;
			
			instance = codec_base_create();
			if(NULL != instance)
			{
				do_codec_base_instantiate(instance, pos_base);
				ret = instance->codec_open(instance, c_attr);
				if(ES_SUCCESS == ret)
				{	
					es_hld = (es_codec_hld) instance;
					*c_hld = es_hld;
					return ret;
				}
				else
				{
					codec_base_destroy(instance);
					es_hld = NULL;
					*c_hld = es_hld;
					return ret;
				}
			}
			else
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] no enough mem to alloc inistant!\n" , __FUNCTION__);
				es_hld = NULL;
				*c_hld = es_hld;
				return ES_FAIL;
			}
		}
	}
	es_hld = NULL;
	*c_hld = es_hld;
	ret = ES_FAIL;

	return ret;
}

/*******************************************************************************
* @function name: es_codec_close    
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
es_error_t es_codec_close(es_codec_hld c_hld)
{
	struct codec_base *pcb = (struct codec_base *) c_hld;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pcb)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if(NULL != pcb->codec_close)
	{
		ret = pcb->codec_close(pcb);
		codec_base_destroy(pcb);
	}
	else
	{
		ret =  ES_INVALID_PARAM;
	}
	return ret;

}

/*******************************************************************************
* @function name: es_codec_get_attr    
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
es_error_t es_codec_get_attr(es_codec_hld c_hld, struct es_codec_attr *c_attr)
{
	es_error_t ret = ES_SUCCESS;
	struct codec_base *pcb = (struct codec_base *) c_hld;

	if((NULL == pcb) || (NULL == c_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	memcpy(c_attr, &pcb->attr, sizeof(struct es_codec_attr));
	ret = ES_SUCCESS;
	return ret;

}

/*******************************************************************************
* @function name: es_codec_decode    
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
es_error_t es_codec_decode(es_codec_hld c_hld, struct es_data_chunk *input_dat,
	struct es_data_frame **out_frame_list, int *frame_count)
{

	es_error_t ret = ES_SUCCESS;
	struct codec_base *pcb = (struct codec_base *) c_hld;

	if((NULL == pcb) || (NULL == input_dat)
	|| (NULL == out_frame_list) || (NULL == frame_count))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

	if(NULL != pcb->codec_decode)
	{
		ret = pcb->codec_decode(pcb, input_dat, out_frame_list, frame_count);
	}
	else
	{
		ret = ES_INVALID_PARAM;
	}
	return ret;

}

/*******************************************************************************
* @function name: es_codec_encode    
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
es_error_t es_codec_encode(es_codec_hld c_hld, struct es_data_frame *input_dat,
	struct es_data_chunk *output_dat, int *got_enc_chunk)
{
	es_error_t ret = ES_SUCCESS;
	struct codec_base *pcb = (struct codec_base *) c_hld;

	if((NULL == pcb) || (NULL == input_dat)
	|| (NULL == output_dat) || (NULL == got_enc_chunk))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

	if(NULL != pcb->codec_encode)
	{
		ret = pcb->codec_encode(pcb, input_dat, output_dat, got_enc_chunk);
	}
	else
	{
		ret = ES_INVALID_PARAM;
	}
	return ret;



}

