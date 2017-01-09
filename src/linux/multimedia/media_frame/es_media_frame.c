/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_media_frame.c 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2016-12-10    
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
#include <es_media_frame.h>
#include <stdlib.h>
#include <string.h>
#include <es_display.h>

/*******************************************************************************
* @function name: es_media_frame_create    
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
struct es_media_frame* es_media_frame_create(void)
{
	struct es_media_frame *pframe = NULL;

	pframe = (struct es_media_frame *)malloc(sizeof(struct es_media_frame));
	if(NULL != pframe)
	{
		pframe->type = ES_UNKNOW_FRAME;
		memset(&pframe->attr, 0x00, sizeof(union es_frame_attr));
		pframe->buf_size = 0;
		pframe->buf_start_addr = NULL;
		INIT_ES_LIST_HEAD(&pframe->entry);	
	}
	else
	{	
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to alloc!\n" , __FUNCTION__);
		pframe = NULL;
	}	
	return pframe;
}
/*******************************************************************************
* @function name: es_media_frame_destroy    
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
void es_media_frame_destroy(struct es_media_frame *pframe)
{
	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return;
	}
	
	if(NULL != pframe->buf_start_addr)
	{
		free(pframe->buf_start_addr);
		pframe->buf_size = 0;
		pframe->buf_start_addr = NULL;
	}
	free(pframe);
	pframe = NULL;
}

/*******************************************************************************
* @function name: es_media_frame_buf_alloc    
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
es_error_t es_media_frame_buf_alloc(struct es_media_frame *pframe, unsigned long buf_size)
{
	unsigned long byte_len = buf_size;
	es_error_t ret = ES_SUCCESS;

	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

	pframe->buf_start_addr = (unsigned char *) malloc(byte_len);
	if(NULL != pframe->buf_start_addr)
	{
		pframe->buf_size = byte_len;
		ret = ES_SUCCESS;
	}
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to alloc!\n" , __FUNCTION__);
		ret = ES_FAIL;
	}
	return ret;
}
/*******************************************************************************
* @function name: es_media_frame_buf_free    
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
void es_media_frame_buf_free(struct es_media_frame *pframe)
{
	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ;
	}

	if(NULL != pframe->buf_start_addr)
	{
		free(pframe->buf_start_addr);
		pframe->buf_size = 0;
		pframe->buf_start_addr = NULL;
	}
}
/*******************************************************************************
* @function name: es_media_frame_buf_realloc    
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
es_error_t es_media_frame_buf_realloc(struct es_media_frame *pframe, unsigned long buf_size)
{
	es_error_t ret = ES_SUCCESS;
	unsigned char *pre_mem = NULL;
	unsigned long pre_buf_size = 0;

	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		ret= ES_INVALID_PARAM;
		return ret;
	}
	if(buf_size == pframe->buf_size)
	{
		ret = ES_SUCCESS;
		return ret;
	}
	
	if(NULL != pframe->buf_start_addr)
	{
		pre_buf_size = pframe->buf_size;
		pre_mem = pframe->buf_start_addr;
		pframe->buf_size = buf_size;
		pframe->buf_start_addr = (unsigned char *) malloc(pframe->buf_size);
		if(NULL != pframe->buf_start_addr)
		{
			free(pre_mem);
			pre_mem = NULL;
			pre_buf_size = 0;
			ret = ES_SUCCESS;
		}
		else
		{
			pframe->buf_size = pre_buf_size;
			pframe->buf_start_addr = pre_mem;
			pre_mem = NULL;
			pre_buf_size = 0;
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] No mem when realloc frame!\n" , __FUNCTION__);
			ret = ES_FAIL;
		}
	}
	else
	{
		ret = es_media_frame_buf_alloc(pframe, buf_size);
	}
	return ret;
}

