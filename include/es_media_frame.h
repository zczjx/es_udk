/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_media_frame.h 
*                
* @author: Clarence.Chow <zhou_chenz@163.com> 
*                
* @version:
*                
* @date: 2016-11-7    
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
#ifndef _ES_MEDIA_FRAME_H_
#define _ES_MEDIA_FRAME_H_
#include <es_common.h>
#include <es_list.h>

typedef int frame_fmt_t;

typedef enum{
	ES_UNKNOW_FRAME = 0,

	ES_PROTOCOL_DATA_FRAME,
	
	ES_PKG_DATA_FRAME,

	ES_COMPRESS_AUDIO_FRAME,
		
	ES_COMPRESS_VIDEO_FRAME,

	ES_COMPRESS_IMAGE_FRAME,

	ES_PIXEL_FRAME,

	ES_SOUND_FRAME,
	
	ES_FRAME_TYPE_NR,
} es_frame_type;

/* protocol frame attr */

typedef struct es_protocol_data_frame_attr{
	unsigned long protocol_type;
} es_protocol_data_frame_attr;

/* packaged frame attr */

typedef struct es_pkg_data_frame_attr{
	unsigned long pkg_fmt;
} es_pkg_data_frame_attr;


/* compress audio frame attr */

typedef struct es_compress_audio_frame_attr{
	unsigned long compress_fmt;
} es_compress_audio_frame_attr;


/* es video_frame_attr compress_fmt */
typedef enum{
	ES_VIDEO_COMPRESS_FMT_UNKNOW = 0,
		
	ES_VIDEO_COMPRESS_FMT_MJPEG,

	ES_VIDEO_COMPRESS_FMT_NR,
} es_video_compress_fmt;



typedef struct es_compress_video_frame_attr{
	es_video_compress_fmt compress_fmt;
} es_compress_video_frame_attr;

/* compress image frame attr */

#define ES_IMAGE_FRAME_COMPRESS_FMT_UNKNOW	0
#define ES_IMAGE_FRAME_COMPRESS_FMT_JPEG	1
#define ES_IMAGE_FRAME_COMPRESS_FMT_PNG		2
#define ES_IMAGE_FRAME_COMPRESS_FMT_BMP		3

typedef struct es_compress_image_frame_attr{
	unsigned long compress_fmt;
} es_compress_image_frame_attr;

/* pixel frame attr */
typedef enum{
	ES_PIX_FMT_UNKNOW = 0,
		
	ES_PIX_FMT_RGB332,
	ES_PIX_FMT_RGB565,
	ES_PIX_FMT_RGB24,
	ES_PIX_FMT_BGRA32,
	
	ES_PIX_FMT_YUYV,
	
	ES_PIX_FMT_NR,
} es_pix_fmt;


typedef struct es_pixel_frame_attr{
	es_pix_fmt 			pix_fmt;
	unsigned long 		x_resolution; 
	unsigned long 		y_resolution; 
	unsigned long 		bpp; // bits per pixel
} es_pixel_frame_attr;

/* sound frame attr */
typedef struct es_sound_frame_attr{
	unsigned long stub;
} es_sound_frame_attr;



typedef union es_frame_attr{
	struct es_protocol_data_frame_attr protocol_dat;
	struct es_pkg_data_frame_attr pkg_dat_attr;
	struct es_compress_audio_frame_attr audio_frame;
	struct es_compress_video_frame_attr video_frame;
	struct es_compress_image_frame_attr image_frame;
	struct es_pixel_frame_attr pix_frame;
	struct es_sound_frame_attr sound_frame;
} es_frame_attr;


typedef struct es_media_frame{
	es_frame_type type;
	union es_frame_attr attr;
	unsigned long buf_size;
	unsigned char *buf_start_addr;
	struct es_list_head entry;
} es_media_frame;


extern struct es_media_frame * es_media_frame_create(void);
extern void es_media_frame_destroy(struct es_media_frame *pframe);

extern es_error_t es_media_frame_buf_alloc(struct es_media_frame *pframe, unsigned long buf_size);
extern void es_media_frame_buf_free(struct es_media_frame *pframe);
extern es_error_t es_media_frame_buf_realloc(struct es_media_frame *pframe, unsigned long buf_size);

#endif /* ifndef _ES_MEDIA_FRAME_H_.2016-11-7 23:20:25 zcz */

