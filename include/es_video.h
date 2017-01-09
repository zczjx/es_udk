/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: es_video.h 
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
#ifndef _ES_VIDEO_H_
#define _ES_VIDEO_H_
#include <es_common.h>
#include <es_media_frame.h>

typedef void * es_video_hld;

typedef enum{
	ES_VIDEO_CLASS_UNKNOW = 0,
	ES_VIDEO_CLASS_CAMERA,
} es_video_class;


/* video_property bit value */
#define ES_VIDEO_PROPERTY_UNKNOW			0x00000000	/* Can capture */
#define ES_VIDEO_PROPERTY_CAPTURE			0x00000001	/* Can capture */
#define ES_VIDEO_PROPERTY_TUNER				0x00000002	/* Can tune */
#define ES_VIDEO_PROPERTY_TELETEXT			0x00000004	/* Does teletext */
#define ES_VIDEO_PROPERTY_OVERLAY			0x00000008	/* Overlay onto frame buffer */
#define ES_VIDEO_PROPERTY_CHROMAKEY			0x00000010	/* Overlay by chromakey */
#define ES_VIDEO_PROPERTY_CLIPPING			0x00000020	/* Can clip */
#define ES_VIDEO_PROPERTY_FRAMERAM			0x00000040	/* Uses the frame buffer memory */
#define ES_VIDEO_PROPERTY_SCALES			0x00000080	/* Scalable */
#define ES_VIDEO_PROPERTY_MONOCHROME		0x00000100	/* Monochrome only */
#define ES_VIDEO_PROPERTY_SUBCAPTURE		0x00000200	/* Can capture subareas of the image */
#define ES_VIDEO_PROPERTY_MPEG_DECODER		0x00000400	/* Can decode MPEG streams */
#define ES_VIDEO_PROPERTY_MPEG_ENCODER		0x00000800	/* Can encode MPEG streams */
#define ES_VIDEO_PROPERTY_MJPEG_DECODER		0x00001000	/* Can decode MJPEG streams */
#define ES_VIDEO_PROPERTY_MJPEG_ENCODER		0x00002000	/* Can encode MJPEG streams */


typedef struct es_video_resolution{
	unsigned long x;
    unsigned long y;
} es_video_resolution;

typedef struct es_video_attr{
	unsigned long property;  //ro
	es_pix_fmt pix_fomat; //ro
	es_video_compress_fmt video_compress_fmt; //ro
	unsigned long bpp; //ro bits per pixel
	struct es_video_resolution resolution; //rw
} es_video_attr;



extern es_error_t es_video_init();

extern es_error_t es_video_deinit();

extern es_error_t es_video_open(const char *path, 
	es_video_class sub_class, 
	es_video_hld *v_hld);

extern es_error_t es_video_close(es_video_hld v_hld);

extern es_error_t es_video_get_attr(es_video_hld v_hld, struct es_video_attr *v_attr);

extern es_error_t es_video_set_attr(es_video_hld v_hld, struct es_video_attr *v_attr);

extern es_error_t es_video_start(es_video_hld v_hld);

extern es_error_t es_video_stop(es_video_hld v_hld);

typedef es_error_t (*video_callback)(struct es_media_frame *vframe, void *arg);

extern es_error_t es_video_async_recv_frame(es_video_hld v_hld, 
	video_callback vcb, 
	void *arg);

extern es_error_t es_video_sync_recv_frame(es_video_hld v_hld, 
	struct es_media_frame *vframe);

extern es_error_t es_video_async_send_frame(es_video_hld v_hld,
	struct es_media_frame *vframe, 
	video_callback vcb, 
	void *arg);

extern es_error_t es_video_sync_send_frame(es_video_hld v_hld, 
	struct es_media_frame *vframe);




#endif /* ifndef _ES_VIDEO_H_.2016-11-7 20:54:34 zcz */

