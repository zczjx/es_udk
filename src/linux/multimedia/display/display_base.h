/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: display_base.h 
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
#ifndef _DISPLAY_BASE_H_
#define _DISPLAY_BASE_H_
#include <es_display.h>


typedef struct display_base {
	es_display_class sub_class;
	const char *path_name;
	int fd;
	struct es_display_attr attr;
	struct es_list_head entry;
	void *priv;

	es_error_t (*display_open)(const char *path, struct display_base *base);
	es_error_t (*display_close)(struct display_base *base);
	es_error_t (*display_on)(struct display_base *base);
	es_error_t (*display_off)(struct display_base *base);
	es_error_t (*display_flush)(struct display_base *base, struct es_data_frame *dframe);
	es_error_t (*display_grab)(struct display_base *base, struct es_data_frame *dframe);
} display_base;

typedef es_error_t (*sub_init)(void);

extern es_error_t display_base_register(struct display_base *pd);


static inline struct display_base *display_base_create()
{
	struct display_base *dbase = NULL;

	dbase = (struct display_base *) malloc(sizeof(struct display_base));
	if(NULL == dbase)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no mem to create display_base!\n" ,__FUNCTION__);
        return NULL;
	}
	memset(dbase, NULL, sizeof(struct display_base));
	return dbase;

}

static inline void display_base_destroy(struct display_base * pd)
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

#endif /* ifndef _DISPLAY_BASE_H_.2016-12-4 22:59:44 zcz */

