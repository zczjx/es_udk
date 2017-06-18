/*******************************************************************************
* Copyright (C), 2000-2017,  Electronic Technology Co., Ltd.
*                
* @filename: es_multimedia_format.c 
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
#include <es_multimedia_format.h>
#include <linux/videodev2.h>
#include <libavutil/pixfmt.h> 
static unsigned long bits_per_pixel_arr[] = {
	[ES_PIX_FMT_UNKNOW] = 0,
		
	[ES_PIX_FMT_GRAY8] = 8,
	[ES_PIX_FMT_GRAY16] = 16,
	
	[ES_PIX_FMT_RGB332] = 8,
	[ES_PIX_FMT_RGB565] = 16,
	[ES_PIX_FMT_RGB24] = 24,
	[ES_PIX_FMT_BGRA32] = 32,
	
	[ES_PIX_FMT_YUYV] = 16,
};


static es_multimedia_fmt_t es_pixel_fmt_to_v4l2_arr[] = {
	[ES_PIX_FMT_UNKNOW] = 0,
		
	[ES_PIX_FMT_GRAY8] = V4L2_PIX_FMT_GREY,
	[ES_PIX_FMT_GRAY16] = V4L2_PIX_FMT_Y16,
	
	[ES_PIX_FMT_RGB332] = V4L2_PIX_FMT_RGB332,
	[ES_PIX_FMT_RGB565] = V4L2_PIX_FMT_RGB565,
	[ES_PIX_FMT_RGB24] = V4L2_PIX_FMT_RGB24,
	[ES_PIX_FMT_BGRA32] = V4L2_PIX_FMT_RGB32,
	
	[ES_PIX_FMT_YUYV] = V4L2_PIX_FMT_YUYV,
};


static es_multimedia_fmt_t es_video_encode_fmt_to_v4l2_arr[] = {
	[ES_VIDEO_ENCODE_FMT_UNKNOW] = 0,
		
	[ES_VIDEO_ENCODE_FMT_MJPEG] = V4L2_PIX_FMT_MJPEG,
	
};

static enum AVPixelFormat es_pixel_fmt_to_ffmpeg_arr[] = {
	[ES_PIX_FMT_UNKNOW] = AV_PIX_FMT_NONE,
		
	[ES_PIX_FMT_RGB332] = AV_PIX_FMT_RGB8,
	[ES_PIX_FMT_RGB565] = AV_PIX_FMT_RGB565LE,
	[ES_PIX_FMT_RGB24]	= AV_PIX_FMT_RGB24,
	[ES_PIX_FMT_BGRA32]	= AV_PIX_FMT_BGRA,
	
	[ES_PIX_FMT_YUYV] = AV_PIX_FMT_YUYV422,
};

/*******************************************************************************
* @function name: es_pix_fmt_to_v4l2_fmt    
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
es_multimedia_fmt_t es_pix_fmt_to_v4l2_fmt(es_pix_fmt fmt)
{
	es_multimedia_fmt_t v4l2_pix_fmt = 0;

	if( (fmt >= ES_PIX_FMT_NR)
	|| (fmt <= ES_PIX_FMT_UNKNOW) )
	{
		v4l2_pix_fmt = 0;
	}
	else
	{
		v4l2_pix_fmt = es_pixel_fmt_to_v4l2_arr[fmt];
	}
	return v4l2_pix_fmt;
}
/*******************************************************************************
* @function name: es_image_encode_fmt_to_v4l2_fmt    
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
es_multimedia_fmt_t es_image_encode_fmt_to_v4l2_fmt(es_image_encode_fmt fmt)
{
	return 0;
	

}
/*******************************************************************************
* @function name: es_video_encode_fmt_to_v4l2_fmt    
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
es_multimedia_fmt_t es_video_encode_fmt_to_v4l2_fmt(es_video_encode_fmt fmt)
{
	es_multimedia_fmt_t v4l2_fmt = 0;

	if( (fmt >= ES_VIDEO_ENCODE_FMT_NR)
	|| (fmt <= ES_VIDEO_ENCODE_FMT_UNKNOW) )
	{
		v4l2_fmt = 0;
	}
	else
	{
		v4l2_fmt = es_video_encode_fmt_to_v4l2_arr[fmt];
	}
	return v4l2_fmt;
}

/*******************************************************************************
* @function name: v4l2_fmt_to_es_pix_fmt    
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
es_pix_fmt v4l2_fmt_to_es_pix_fmt(es_multimedia_fmt_t fmt)
{
	es_pix_fmt ret = ES_PIX_FMT_UNKNOW;
	
	switch(fmt)
	{
		case V4L2_PIX_FMT_GREY:
			ret = ES_PIX_FMT_GRAY8;
			break;

		case V4L2_PIX_FMT_Y16:
			ret = ES_PIX_FMT_GRAY16;
			break;

		case V4L2_PIX_FMT_YUYV:
			ret = ES_PIX_FMT_YUYV;
			break;

		case V4L2_PIX_FMT_RGB332:
			ret = ES_PIX_FMT_RGB332;
			break;

		case V4L2_PIX_FMT_RGB565:
			ret = ES_PIX_FMT_RGB565;
			break;
			
		case V4L2_PIX_FMT_RGB24:
			ret = ES_PIX_FMT_RGB24;
			break;

		case V4L2_PIX_FMT_RGB32:
			ret = ES_PIX_FMT_BGRA32;
			break;
			
		default:
			ret = ES_PIX_FMT_UNKNOW;
			break;
	}
	return ret;

}
/*******************************************************************************
* @function name: v4l2_fmt_to_es_image_encode_fmt    
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
es_image_encode_fmt v4l2_fmt_to_es_image_encode_fmt(es_multimedia_fmt_t fmt)
{

}
/*******************************************************************************
* @function name: v4l2_fmt_to_es_video_encode_fmt    
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
es_video_encode_fmt v4l2_fmt_to_es_video_encode_fmt(es_multimedia_fmt_t fmt)
{
	es_video_encode_fmt ret = ES_VIDEO_ENCODE_FMT_UNKNOW;
	
	switch(fmt)
	{
		/* compressed format*/
		case V4L2_PIX_FMT_MJPEG:
			ret = ES_VIDEO_ENCODE_FMT_MJPEG;
			break;

		default:
			ret = ES_VIDEO_ENCODE_FMT_UNKNOW;
			break;
	}
	return ret;

}

/*******************************************************************************
* @function name: es_pix_fmt_to_ffmpeg_fmt    
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
es_multimedia_fmt_t es_pix_fmt_to_ffmpeg_fmt(es_pix_fmt fmt)
{
	es_multimedia_fmt_t ffmpeg_pix_fmt = 0;

	if( (fmt >= ES_PIX_FMT_NR)
	|| (fmt <= ES_PIX_FMT_UNKNOW) )
	{
		ffmpeg_pix_fmt = 0;
	}
	else
	{
		ffmpeg_pix_fmt = es_pixel_fmt_to_ffmpeg_arr[fmt];
	}
	return ffmpeg_pix_fmt;

}
/*******************************************************************************
* @function name: es_image_encode_fmt_to_ffmpeg_fmt    
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
es_multimedia_fmt_t es_image_encode_fmt_to_ffmpeg_fmt(es_image_encode_fmt fmt)
{

}
/*******************************************************************************
* @function name: es_video_encode_fmt_to_ffmpeg_fmt    
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
es_multimedia_fmt_t es_video_encode_fmt_to_ffmpeg_fmt(es_video_encode_fmt fmt)
{

}
/*******************************************************************************
* @function name: es_audio_encode_fmt_to_ffmpeg_fmt    
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
es_multimedia_fmt_t es_audio_encode_fmt_to_ffmpeg_fmt(es_audio_encode_fmt fmt)
{

}

/*******************************************************************************
* @function name: ffmpeg_fmt_to_es_pix_fmt    
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
es_pix_fmt ffmpeg_fmt_to_es_pix_fmt(es_multimedia_fmt_t fmt)
{

}
/*******************************************************************************
* @function name: ffmpeg_fmt_to_es_image_encode_fmt    
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
es_image_encode_fmt ffmpeg_fmt_to_es_image_encode_fmt(es_multimedia_fmt_t fmt)
{

}
/*******************************************************************************
* @function name: ffmpeg_fmt_to_es_video_encode_fmt    
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
es_video_encode_fmt ffmpeg_fmt_to_es_video_encode_fmt(es_multimedia_fmt_t fmt)
{

}
/*******************************************************************************
* @function name: ffmpeg_fmt_to_es_audio_encode_fmt    
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
es_audio_encode_fmt ffmpeg_fmt_to_es_audio_encode_fmt(es_multimedia_fmt_t fmt)
{

}

/*******************************************************************************
* @function name: es_pix_fmt_bits_per_pixel    
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
unsigned long es_pix_fmt_bits_per_pixel(es_pix_fmt fmt)
{
	unsigned long ret_bpp = 0;

	if( (fmt >= ES_PIX_FMT_NR)
	|| (fmt <= ES_PIX_FMT_UNKNOW) )
	{
		ret_bpp = 0;
	}
	else
	{
		ret_bpp = bits_per_pixel_arr[fmt];
	}
	return ret_bpp;
}

/*******************************************************************************
* @function name: es_pix_fmt_bytes_per_pixel    
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
unsigned long es_pix_fmt_bytes_per_pixel(es_pix_fmt fmt)
{
	return 0;

}
