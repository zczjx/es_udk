/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: es_data_frame.c 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2017-6-17    
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
#include <es_data_frame.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



/*******************************************************************************
* @function name: es_data_frame_create    
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
struct es_data_frame * es_data_frame_create(void)
{
	struct es_data_frame *pframe = NULL;

	pframe = (struct es_data_frame *)malloc(sizeof(struct es_data_frame));
	if(NULL != pframe)
	{
		pframe->type = ES_UNKNOW_FRAME;
		memset(&pframe->frame_attr, 0x00, sizeof(union es_frame_attr));
		memset(&pframe->mem_mmap_attr, 0x00, sizeof(struct data_frame_mmap_attr));
		pframe->mem_mmap_attr.fd = -1;
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
* @function name: es_data_frame_destroy    
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
void es_data_frame_destroy(struct es_data_frame *pframe)
{
	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return;
	}
	switch(pframe->mem_method)
	{
		case DATA_FRAME_MEM_METHOD_FILE_MMAP:
			if(NULL != pframe->buf_start_addr)
			{
				munmap(pframe->buf_start_addr, pframe->buf_size);
				pframe->buf_start_addr = NULL;
				pframe->buf_size = 0;
			}
			if(pframe->mem_mmap_attr.fd >= 0)
			{
				close(pframe->mem_mmap_attr.fd);
				pframe->mem_mmap_attr.fd = -1;
				pframe->mem_mmap_attr.offset = 0;
				remove(pframe->mem_mmap_attr.mmap_file_path);
				free(pframe->mem_mmap_attr.mmap_file_path);
				pframe->mem_mmap_attr.mmap_file_path = NULL;
			}
			break;

		case DATA_FRAME_MEM_METHOD_MALLOC:
			if(NULL != pframe->buf_start_addr)
			{
				free(pframe->buf_start_addr);
				pframe->buf_size = 0;
				pframe->buf_start_addr = NULL;
			}
			break;

		default:

			break;
	}
	
	free(pframe);
	pframe = NULL;
}

/*******************************************************************************
* @function name: es_data_frame_buf_alloc    
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
es_error_t es_data_frame_buf_alloc(struct es_data_frame *pframe, 
	data_frame_memory_method mem_method, 
	unsigned long buf_size)
{
	unsigned long byte_len = buf_size;
	es_error_t ret = ES_SUCCESS;
	char *cwd;
	struct timespec tp;
	char t_stamp[1 + (sizeof(long) * 4 * 2 * 2) + 1];
	char *abs_file_path;
	int fd;
			


	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

	switch(mem_method)
	{
		case DATA_FRAME_MEM_METHOD_FILE_MMAP:
			cwd = get_current_dir_name();
			clock_gettime(CLOCK_REALTIME, &tp);
			sprintf(t_stamp, ".%d%d", tp.tv_sec, tp.tv_nsec);
			abs_file_path = malloc(strlen(cwd) + 1 + 1 + strlen(t_stamp) + 1);
			if(NULL == abs_file_path)
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] no mem to alloc!\n" , __FUNCTION__);
				ret = ES_FAIL;
			}
			strcat(abs_file_path, "/");
			strcat(abs_file_path, t_stamp);
			fd = open(abs_file_path, O_RDWR | O_CREAT);
    		if (fd < 0)
    		{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s]can not open %s\n" , __FUNCTION__, abs_file_path);
        		return ES_FAIL;
    		}
			pframe->buf_start_addr = mmap(0, buf_size, 
										(PROT_READ | PROT_WRITE), MAP_SHARED, 
        			  					fd, 0);
			if (MAP_FAILED == pframe->buf_start_addr) 
            {
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] Unable to map buffer.\n" , __FUNCTION__);
				return ES_FAIL;
        	}
			
			pframe->mem_method = DATA_FRAME_MEM_METHOD_FILE_MMAP;
			pframe->mem_mmap_attr.mmap_file_path = abs_file_path;
			pframe->mem_mmap_attr.fd = fd;
			pframe->mem_mmap_attr.offset = 0;
			pframe->buf_size = buf_size;
			ret = ES_SUCCESS;
			break;

		case DATA_FRAME_MEM_METHOD_MALLOC:
			pframe->buf_start_addr = (unsigned char *) malloc(byte_len);
			if(NULL != pframe->buf_start_addr)
			{
				pframe->mem_method = DATA_FRAME_MEM_METHOD_FILE_MMAP;
				pframe->buf_size = byte_len;
				ret = ES_SUCCESS;
			}
			else
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] no mem to alloc!\n" , __FUNCTION__);
				ret = ES_FAIL;
			}
			break;

		default:

			break;
	}
	
	return ret;
}
/*******************************************************************************
* @function name: es_data_frame_buf_free    
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
void es_data_frame_buf_free(struct es_data_frame *pframe)
{
	if(NULL == pframe)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ;
	}

	switch(pframe->mem_method)
	{
		case DATA_FRAME_MEM_METHOD_FILE_MMAP:
			if(NULL != pframe->buf_start_addr)
			{
				munmap(pframe->buf_start_addr, pframe->buf_size);
				pframe->buf_start_addr = NULL;
				pframe->buf_size = 0;
			}
			if(pframe->mem_mmap_attr.fd >= 0)
			{
				close(pframe->mem_mmap_attr.fd);
				pframe->mem_mmap_attr.fd = -1;
				pframe->mem_mmap_attr.offset = 0;
				remove(pframe->mem_mmap_attr.mmap_file_path);
				free(pframe->mem_mmap_attr.mmap_file_path);
				pframe->mem_mmap_attr.mmap_file_path = NULL;
			}
			break;

		case DATA_FRAME_MEM_METHOD_MALLOC:
			if(NULL != pframe->buf_start_addr)
			{
				free(pframe->buf_start_addr);
				pframe->mem_method = DATA_FRAME_MEM_METHOD_UNKNOW;
				pframe->buf_size = 0;
				pframe->buf_start_addr = NULL;
			}
			break;

		default:

			break;
	}

}
/*******************************************************************************
* @function name: es_data_frame_buf_realloc    
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
es_error_t es_data_frame_buf_realloc(struct es_data_frame *pframe, unsigned long buf_size)
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
		switch(pframe->mem_method)
		{
			case DATA_FRAME_MEM_METHOD_FILE_MMAP:
				pre_buf_size = pframe->buf_size;
				pre_mem = pframe->buf_start_addr;
				pframe->buf_size = buf_size;
				pframe->buf_start_addr = mmap(0, buf_size, 
										(PROT_READ | PROT_WRITE), MAP_SHARED, 
        			  					pframe->mem_mmap_attr.fd, 0);
				if (MAP_FAILED == pframe->buf_start_addr) 
            	{
					pframe->buf_size = pre_buf_size;
					pframe->buf_start_addr = pre_mem;
					pre_mem = NULL;
					pre_buf_size = 0;
					ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
					ES_PRINTF("[%s] No mem when realloc frame!\n" , __FUNCTION__);
					ret = ES_FAIL;
        		}
				pframe->mem_method = DATA_FRAME_MEM_METHOD_FILE_MMAP;
				pframe->mem_mmap_attr.offset = 0;
				pframe->buf_size = buf_size;
				munmap(pre_mem, pre_buf_size);
				pre_mem = NULL;
				pre_buf_size = 0;
				ret = ES_SUCCESS;
			
			break;

		case DATA_FRAME_MEM_METHOD_MALLOC:
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
					pframe->mem_method = DATA_FRAME_MEM_METHOD_MALLOC;
					pre_mem = NULL;
					pre_buf_size = 0;
					ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
					ES_PRINTF("[%s] No mem when realloc frame!\n" , __FUNCTION__);
					ret = ES_FAIL;
				}
			break;
		default:
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] pframe->mem_method is incorrect!\n" , __FUNCTION__);
			break;
		}
	}
	else
	{
		ret = es_data_frame_buf_alloc(pframe, pframe->mem_method, buf_size);
	}
	
	return ret;
}