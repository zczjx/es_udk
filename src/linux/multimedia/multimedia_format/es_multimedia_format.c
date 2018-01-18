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
#include <libavcodec/avcodec.h>

static struct es_pix_fmt_info pix_fmt_info_arr[] = {
	[ES_PIX_FMT_UNKNOW] = {
		.store_fmt = ES_PIX_STORE_FMT_UNKNOW,
		.bits_per_pix = 0,
	},
		
	[ES_PIX_FMT_GRAY8] ={
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 8,
	},
	
	[ES_PIX_FMT_GRAY16] = {
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 16,
	},
	
	[ES_PIX_FMT_RGB332] = {
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 8,
	},
	[ES_PIX_FMT_RGB565] = {
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 16,
	},
	[ES_PIX_FMT_RGB24] = {
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 24,
	},
	[ES_PIX_FMT_BGRA32] = {
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 32,
	},
	
	[ES_PIX_FMT_YUYV] = {
		.store_fmt = ES_PIX_STORE_FMT_PACKED,
		.bits_per_pix = 16,
	},
	[ES_PIX_FMT_YUVJ422P] = {
		.store_fmt = ES_PIX_STORE_FMT_PLANAR,
		.bits_per_pix = 16,
	},
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
	[ES_PIX_FMT_YUVJ422P] = AV_PIX_FMT_YUVJ422P,
};

static enum AVCodecID es_video_encode_fmt_to_ffmpeg_arr[] = {
	[ES_VIDEO_ENCODE_FMT_UNKNOW] = AV_CODEC_ID_NONE,

	[ES_VIDEO_ENCODE_FMT_MJPEG] = AV_CODEC_ID_MJPEG,
	[ES_VIDEO_ENCODE_FMT_H264] = AV_CODEC_ID_H264,		
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
		ffmpeg_pix_fmt = AV_PIX_FMT_NONE;
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

	es_multimedia_fmt_t ffmpeg_pix_fmt = 0;

	if( (fmt >= ES_VIDEO_ENCODE_FMT_NR)
	|| (fmt <= ES_VIDEO_ENCODE_FMT_UNKNOW) )
	{
		ffmpeg_pix_fmt = AV_CODEC_ID_NONE;
	}
	else
	{
		ffmpeg_pix_fmt = es_video_encode_fmt_to_ffmpeg_arr[fmt];
	}
	return ffmpeg_pix_fmt;

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

	es_pix_fmt ret = ES_PIX_FMT_UNKNOW;
	
	switch(fmt)
	{
		case AV_PIX_FMT_NONE:
			ret = ES_PIX_FMT_UNKNOW;
			break;

		case AV_PIX_FMT_RGB8:
			ret = ES_PIX_FMT_RGB332;
			break;

		case AV_PIX_FMT_RGB565LE:
			ret = ES_PIX_FMT_RGB565;
			break;

		case AV_PIX_FMT_RGB24:
			ret = ES_PIX_FMT_RGB24;
			break;

		case AV_PIX_FMT_BGRA:
			ret = ES_PIX_FMT_BGRA32;
			break;
			
		case AV_PIX_FMT_YUYV422:
			ret = ES_PIX_FMT_YUYV;
			break;

		case AV_PIX_FMT_YUVJ422P:
			ret = ES_PIX_FMT_YUVJ422P;
			break;
			
		default:
			ret = ES_PIX_FMT_UNKNOW;
			break;
	}
	return ret;

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
* @function name: es_get_pix_fmt_info    
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
es_error_t es_get_pix_fmt_info(es_pix_fmt fmt, struct es_pix_fmt_info *pinfo)
{
	es_error_t ret = ES_SUCCESS;

	if(NULL == pinfo)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}

	if((fmt >= ES_PIX_FMT_NR)
	|| (fmt <= ES_PIX_FMT_UNKNOW))
	{
		pinfo->store_fmt = ES_PIX_STORE_FMT_UNKNOW;
		pinfo->bits_per_pix = 0;	
	}
	else
	{
		pinfo->store_fmt = pix_fmt_info_arr[fmt].store_fmt;
		pinfo->bits_per_pix = pix_fmt_info_arr[fmt].bits_per_pix;
	}

	ret = ES_SUCCESS;
	return ret;
}
