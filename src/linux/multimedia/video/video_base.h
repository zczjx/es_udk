/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: video_base.h 
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
#ifndef _VIDEO_BASE_H_
#define _VIDEO_BASE_H_
#include <es_list.h>
#include <es_video.h>

#include <stdlib.h>
#include <string.h>

typedef enum {
	VBUF_FREE = 0,	
	VBUF_USED,
	VBUF_UNKNOW,
} vbuf_state;

typedef struct video_buf{
	vbuf_state state;
	unsigned long buf_bytes;
	void *start_addr;
	void *planes[ES_DATA_FRAME_MAX_PLANE_NR];
	unsigned long plane_bytes[ES_DATA_FRAME_MAX_PLANE_NR];
	struct es_list_head entry;
} video_buf;

typedef struct video_base{
	es_video_class sub_class;
	const char *path_name;
	int fd;
	struct es_video_attr attr;
	struct es_list_head ctrl_list_head;
	struct es_list_head entry;
	struct es_list_head video_buf_head;
	void *priv;
	es_error_t (*video_open)(const char *path, struct video_base *base);
	es_error_t (*video_close)(struct video_base *base);
	es_error_t (*video_start)(struct video_base *base);
	es_error_t (*video_stop)(struct video_base *base);
	es_error_t (*video_get_attr)(struct video_base *base, struct es_video_attr *public_attr);
	es_error_t (*video_set_attr)(struct video_base *base, struct es_video_attr *public_attr);
	es_error_t (*video_get_ctrl)(struct video_base *base, struct es_video_ctrl_cmd *cmd);
	es_error_t (*video_set_ctrl)(struct video_base *base, struct es_video_ctrl_cmd *cmd);
	es_error_t (*video_send_frame)(struct video_base *base, struct es_data_frame *vframe);
	es_error_t (*video_recv_frame)(struct video_base *base, struct es_data_frame *vframe);
	es_error_t (*video_recv_encode_chunk)(struct video_base *base, struct es_data_chunk *vchunk);
} video_base;


typedef es_error_t (*sub_init)();

extern es_error_t video_base_register(struct video_base *pv);


/*******************************************************************************
* @function name: video_base_create    
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
static inline struct video_base *video_base_create()
{
	struct video_base *vbase = NULL;

	vbase = (struct video_base *) malloc(sizeof(struct video_base));
	if(NULL == vbase)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to create video_base!\n" ,__FUNCTION__);
        return NULL;
	}
	memset(vbase, NULL, sizeof(struct video_base));
	return vbase;
}

/*******************************************************************************
* @function name: video_base_destroy    
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
static inline void video_base_destroy(struct video_base * pv)
{
	if(NULL == pv)
	{
        return;
	}
	if(NULL != pv->priv)
	{
		free(pv->priv);
		pv->priv = NULL;
	}
	free(pv);
	pv = NULL;
}

/*******************************************************************************
* @function name: video_buf_create    
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
static inline struct video_buf *video_buf_create()
{
	struct video_buf *vbuf = NULL;

	vbuf  = (struct video_buf *) malloc(sizeof(struct video_buf));
	if(NULL == vbuf)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to create video_buf!\n" ,__FUNCTION__);
        return NULL;
	}
	memset(vbuf, NULL, sizeof(struct video_buf));
	return vbuf;

}

/*******************************************************************************
* @function name: video_buf_destroy    
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
static inline void video_buf_destroy(struct video_buf * pv)
{
	if(NULL == pv)
	{
        return;
	}
	if(NULL != pv->start_addr)
	{
		free(pv->start_addr);
		pv->start_addr = NULL;
	}
	free(pv);
	pv = NULL;
}



#endif /* ifndef _VIDEO_BASE_H_.2016-11-11 21:28:59 zcz */

