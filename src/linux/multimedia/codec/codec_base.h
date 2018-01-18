/*******************************************************************************
* Copyright (C), 2000-2018,  Electronic Technology Co., Ltd.
*                
* @filename: codec_base.h 
*                
* @author: Clarence.Zhou <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2018-1-12    
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
#ifndef _CODEC_BASE_H_
#define _CODEC_BASE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <es_codec.h>

typedef struct codec_base {
	es_codec_class sub_class;
	struct es_codec_attr attr;
	struct es_list_head entry;
	void *priv;
	void *decode_priv_desc;
	void *encode_priv_desc;
	
	bool (*is_codec_support)(es_codec_class codec_class, union es_codec_fmt codec_fmt, long codec_cap);
	
	es_error_t (*codec_open)(struct codec_base *base, struct es_codec_attr *c_attr);
	
	es_error_t (*codec_close)(struct codec_base *base);
	
	es_error_t (*codec_decode)(struct codec_base *base, struct es_data_chunk *input_dat,
		struct es_data_frame **out_frame_list, int *frame_count);

	es_error_t (*codec_encode)(struct codec_base *base, struct es_data_frame *input_dat,
		struct es_data_chunk *output_dat, int *got_chunk);

} codec_base;

typedef es_error_t (*sub_init)(void);

extern es_error_t codec_base_register(struct codec_base *pcodec);


static inline struct codec_base *codec_base_create()
{
	struct codec_base *dbase = NULL;

	dbase = (struct codec_base *) malloc(sizeof(struct codec_base));
	if(NULL == dbase)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to create codec_base!\n" ,__FUNCTION__);
        return NULL;
	}
	memset(dbase, NULL, sizeof(struct codec_base));
	return dbase;

}

static inline void codec_base_destroy(struct codec_base * pd)
{
	if(NULL == pd)
	{
        return;
	}
	if(NULL != pd->priv)
	{
		free(pd->priv);
		pd->priv = NULL;
	}
	free(pd);
	pd = NULL;
}


#ifdef __cplusplus
}
#endif

#endif /* ifndef _CODEC_BASE_H_.2018-1-12 22:24:46 zcz */

