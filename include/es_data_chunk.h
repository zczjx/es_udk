/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: es_data_chunk.h 
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
#ifndef _ES_DATA_CHUNK_H_
#define _ES_DATA_CHUNK_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <es_common.h>
#include <es_list.h>
#include <es_multimedia_format.h>

typedef enum {
	ES_UNKNOW_CHUNK = 0,

	ES_ENCODE_AUDIO_CHUNK,
		
	ES_ENCODE_VIDEO_CHUNK,

	ES_ENCODE_IMAGE_CHUNK,
	
	ES_CHUNK_TYPE_NR,
} es_chunk_type;

typedef union es_chunk_attr {
	struct es_encode_audio_attr encode_audio;
	struct es_encode_video_attr encode_video;
	struct es_encode_image_attr encode_image;
} es_chunk_attr;

typedef enum {
	DATA_CHUNK_MEM_METHOD_UNKNOW = 0,
	DATA_CHUNK_MEM_METHOD_FILE_MMAP,
	DATA_CHUNK_MEM_METHOD_MALLOC,
	DATA_CHUNK_MEM_METHOD_STAIC_ARRAY,
} data_chunk_memory_method;

/*only mem_method is DATA_CHUNK_MEM_METHOD_FILE_MMAP this attr is useful*/
typedef struct data_chunk_mmap_attr {
	char *mmap_file_path;
	int fd;
	unsigned long offset;
} data_chunk_mmap_attr;

typedef struct es_data_chunk {
	es_chunk_type type;
	union es_chunk_attr chunk_attr;
	data_chunk_memory_method mem_method;
	struct data_chunk_mmap_attr mem_mmap_attr;
	unsigned long buf_size;
	void *buf_start_addr;
	struct es_list_head entry;
} es_data_chunk;

extern struct es_data_chunk * es_data_chunk_create(void);

extern void es_data_chunk_destroy(struct es_data_chunk *pchunk);

extern es_error_t es_data_chunk_buf_alloc(struct es_data_chunk *pchunk, 
		data_chunk_memory_method mem_method, 
		unsigned long buf_size);
extern void es_data_chunk_buf_free(struct es_data_chunk *pchunk);
extern es_error_t es_data_chunk_buf_realloc(struct es_data_chunk *pchunk, unsigned long buf_size);


#ifdef __cplusplus
}
#endif

#endif /* ifndef _ES_DATA_CHUNK_H_.2017-6-17 12:49:17 zcz */




