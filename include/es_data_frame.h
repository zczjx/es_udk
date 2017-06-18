/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: es_data_frame.h 
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
#ifndef _ES_DATA_FRAME_H_
#define _ES_DATA_FRAME_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <es_common.h>
#include <es_list.h>
#include <es_multimedia_format.h>

typedef enum{
	ES_UNKNOW_FRAME = 0,

	ES_ENCODE_AUDIO_FRAME,
		
	ES_ENCODE_VIDEO_FRAME,

	ES_ENCODE_IMAGE_FRAME,

	ES_PIXEL_FRAME,

	ES_PCM_SOUND_FRAME,
	
	ES_FRAME_TYPE_NR,
} es_frame_type;

typedef union es_frame_attr{
	struct es_encode_audio_attr encode_audio;
	struct es_pcm_sound_attr pcm_sound;
	struct es_encode_video_attr encode_video;
	struct es_encode_image_attr encode_image;
	struct es_pixel_frame_attr pix_frame;
} es_frame_attr;

typedef enum {
	DATA_FRAME_MEM_METHOD_UNKNOW = 0,
	DATA_FRAME_MEM_METHOD_FILE_MMAP,
	DATA_FRAME_MEM_METHOD_MALLOC,
	DATA_FRAME_MEM_METHOD_STAIC_ARRAY,
} data_frame_memory_method;

/*only mem_method is DATA_FRAME_MEM_METHOD_FILE_MMAP this attr is useful*/
typedef struct data_frame_mmap_attr{
	char *mmap_file_path;
	int fd;
	unsigned long offset;
} data_frame_mmap_attr;

typedef struct es_data_frame{
	es_frame_type type;
	union es_frame_attr frame_attr;
	data_frame_memory_method mem_method;
	struct data_frame_mmap_attr mem_mmap_attr;
	void *buf_start_addr;
	unsigned long buf_size;
	struct es_list_head entry;
} es_data_frame;

extern struct es_data_frame * es_data_frame_create(void);

extern void es_data_frame_destroy(struct es_data_frame *pframe);

extern es_error_t es_data_frame_buf_alloc(struct es_data_frame *pframe, 
		data_frame_memory_method mem_method, 
		unsigned long buf_size);
extern void es_data_frame_buf_free(struct es_data_frame *pframe);
extern es_error_t es_data_frame_buf_realloc(struct es_data_frame *pframe, unsigned long buf_size);


#ifdef __cplusplus
}
#endif

#endif /* ifndef _ES_DATA_FRAME_H_.2017-6-17 10:29:30 zcz */















