/*******************************************************************************
* Copyright (C), 2000-2016,  Electronic Technology Co., Ltd.
*                
* @filename: usb_camera.c 
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
#include <es_common.h>
#include <es_video.h>
#include "video_base.h"
#include <es_media_frame.h>


#include <linux/videodev2.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#define COUNT_OF_V4L2_REQ_BUFFER (4)
#define DEFAULT_V4L2_RESOLUTION_X (480)
#define DEFAULT_V4L2_RESOLUTION_Y (720)
#define V4L2_CAMERA_MAX_CTRL (32)


/*internal self define type*/
typedef struct _v4l2_priv_attr{
	struct v4l2_capability _v4l2_cap;
    struct v4l2_requestbuffers _v4l2_req_bufs;
	int _max_vbuf_bytes;
	int _vbuf_cnt_total;
	int _current_buf_idx;
	struct video_buf *_vbuf_arr[COUNT_OF_V4L2_REQ_BUFFER];
} _v4l2_priv_attr;

/*global variable*/

static const int compatible_pixel_fmt[] = {
	/* displayable pixel format */
	V4L2_PIX_FMT_YUYV, 
	V4L2_PIX_FMT_RGB565,
	V4L2_PIX_FMT_RGB332,
	V4L2_PIX_FMT_RGB24,
	V4L2_PIX_FMT_RGB32,
};

static const int compress_video_fmt[] = {
	/*below is compress video format*/
	V4L2_PIX_FMT_MJPEG,
};


#define VIDEO_BUF_MAX_LEN(resolution) (resolution.x \
	* resolution.y \
	* 4 \
)


/*private method*/
static es_error_t v4l2_cmr_open(const char *path, struct video_base *base);
static es_error_t v4l2_cmr_close(struct video_base *base);
static es_error_t v4l2_cmr_start(struct video_base *base);
static es_error_t v4l2_cmr_stop(struct video_base *base);
static es_error_t v4l2_cmr_get_attr(struct video_base *base, struct es_video_attr *public_attr);
static es_error_t v4l2_cmr_set_attr(struct video_base *base, struct es_video_attr *public_attr);
static es_error_t v4l2_cmr_get_ctrl(struct video_base *base, struct es_video_ctrl_cmd *cmd);
static es_error_t v4l2_cmr_set_ctrl(struct video_base *base, struct es_video_ctrl_cmd *cmd);
static es_error_t v4l2_cmr_send_frame(struct video_base *base, struct es_media_frame *vframe);
static es_error_t v4l2_cmr_recv_frame(struct video_base *base, struct es_media_frame *vframe);

/*internal function */

/*******************************************************************************
* @function name: is_support_pixel_fmt    
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
static inline bool is_support_pixel_fmt(int pix_fmt)
{
	int i;
	int arr_size;
	
	if((0 != sizeof(compatible_pixel_fmt)) && (0 != sizeof(compatible_pixel_fmt)[0]))
	{
		/*escape divide zero*/
		arr_size = sizeof(compatible_pixel_fmt)/sizeof(compatible_pixel_fmt[0]);
	}
	else
	{
		arr_size = 0;
	}
	
    for (i = 0; i < arr_size; i++)
    {
        if (compatible_pixel_fmt[i] == pix_fmt)
            return es_true;
    }
    return es_false;
}

/*******************************************************************************
* @function name: is_support_compress_video_fmt    
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
static inline bool is_support_compress_video_fmt(int video_fmt)
{
	int i;
	int arr_size;
	
	if((0 != sizeof(compress_video_fmt)) && (0 != sizeof(compress_video_fmt)[0]))
	{
		/*escape divide zero*/
		arr_size = sizeof(compress_video_fmt)/sizeof(compress_video_fmt[0]);
	}
	else
	{
		arr_size = 0;
	}
	
    for (i = 0; i < arr_size; i++)
    {
        if (compress_video_fmt[i] == video_fmt)
            return es_true;
    }
    return es_false;
}



/*******************************************************************************
* @function name: pixel_fmt_unify_trans    
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
static inline unsigned long pixel_fmt_unify_trans(__u32 v4l2_fmt)
{
	unsigned long ret = ES_PIX_FMT_UNKNOW;
	
	switch(v4l2_fmt)
	{
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
* @function name: compress_video_fmt_unify_trans    
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
static inline unsigned long compress_video_fmt_unify_trans(__u32 v4l2_fmt)
{
	unsigned long ret = ES_VIDEO_COMPRESS_FMT_UNKNOW;
	
	switch(v4l2_fmt)
	{
		/* compressed format*/
		case V4L2_PIX_FMT_MJPEG:
			ret = ES_VIDEO_COMPRESS_FMT_MJPEG;
			break;

		default:
			ret = ES_VIDEO_COMPRESS_FMT_UNKNOW;
			break;
	}
	return ret;
}


/*******************************************************************************
* @function name: es_video_to_v4l2_pixel_fmt    
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
static inline unsigned long es_video_to_v4l2_pixel_fmt(__u32 es_video_fmt)
{
	unsigned long ret = ES_PIX_FMT_UNKNOW;
	
	switch(es_video_fmt)
	{
		case ES_PIX_FMT_YUYV:
			ret = V4L2_PIX_FMT_YUYV;
			break;

		case ES_PIX_FMT_RGB332:
			ret = V4L2_PIX_FMT_RGB332;
			break;

		case ES_PIX_FMT_RGB565:
			ret = V4L2_PIX_FMT_RGB565;
			break;
			
		case ES_PIX_FMT_RGB24:
			ret = V4L2_PIX_FMT_RGB24;
			break;

		case ES_PIX_FMT_BGRA32:
			ret = V4L2_PIX_FMT_RGB32;
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}


/*******************************************************************************
* @function name: es_video_to_v4l2_compress_fmt    
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
static inline unsigned long es_video_to_v4l2_compress_fmt(__u32 es_video_fmt)
{
	unsigned long ret = 0;
	
	switch(es_video_fmt)
	{
		/* compressed format*/
		case ES_VIDEO_COMPRESS_FMT_MJPEG:
			ret = V4L2_PIX_FMT_MJPEG;
			break;

		default:
			ret = 0;
			break;
	}
	return ret;
}

/*******************************************************************************
* @function name: set_v4l2_fmt_attr    
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
static inline es_error_t set_v4l2_fmt_attr(int fd, struct _v4l2_priv_attr *priv_attr, 
	struct es_video_attr *public_attr)
{
	int err;
	es_error_t ret = ES_SUCCESS;
	struct v4l2_fmtdesc fmt_dsc;
	struct v4l2_format  v4l2_pix_fmt;
	
	memset(&fmt_dsc, 0, sizeof(struct v4l2_fmtdesc));
	fmt_dsc.index = 0;
	fmt_dsc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	public_attr->pix_fomat = ES_PIX_FMT_UNKNOW;
	public_attr->bpp = 0;
	while ((err = ioctl(fd, VIDIOC_ENUM_FMT, &fmt_dsc)) == 0) 
	{
        if (is_support_pixel_fmt(fmt_dsc.pixelformat))
        {
			public_attr->video_compress_fmt = ES_VIDEO_COMPRESS_FMT_UNKNOW;
			public_attr->pix_fomat = pixel_fmt_unify_trans(fmt_dsc.pixelformat);
			public_attr->bpp = video_base_get_bpp(public_attr->pix_fomat);
            break;
        }
		else if(is_support_compress_video_fmt(fmt_dsc.pixelformat))
		{
			public_attr->video_compress_fmt = compress_video_fmt_unify_trans(fmt_dsc.pixelformat);
			public_attr->pix_fomat = ES_PIX_FMT_UNKNOW;
			public_attr->bpp = 0;
            break;
		}
		fmt_dsc.index++;
	}
    if ((ES_PIX_FMT_UNKNOW == public_attr->pix_fomat)
	&& (ES_VIDEO_COMPRESS_FMT_UNKNOW == public_attr->video_compress_fmt))
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] can not support the format of this device\n" , __FUNCTION__);
        return ES_FAIL;        
    }
    /* enum and try format */
    memset(&v4l2_pix_fmt, 0, sizeof(struct v4l2_format));
    v4l2_pix_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_pix_fmt.fmt.pix.pixelformat = fmt_dsc.pixelformat;
    v4l2_pix_fmt.fmt.pix.width       = DEFAULT_V4L2_RESOLUTION_X;
	v4l2_pix_fmt.fmt.pix.height      = DEFAULT_V4L2_RESOLUTION_Y;
    v4l2_pix_fmt.fmt.pix.field       = V4L2_FIELD_ANY;
	/*driver will fix incorrect param and return to user space*/
    err = ioctl(fd, VIDIOC_S_FMT, &v4l2_pix_fmt); 
    if (err) 
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to set format\n" , __FUNCTION__);
        return ES_FAIL;        
    }
    public_attr->resolution.x = v4l2_pix_fmt.fmt.pix.width;
    public_attr->resolution.y = v4l2_pix_fmt.fmt.pix.height;
	ret = ES_SUCCESS;
	return ret;
}


/*******************************************************************************
* @function name: set_v4l2_buf_attr    
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
static inline es_error_t set_v4l2_buf_attr(int fd, 
	struct _v4l2_priv_attr *priv_attr, 
	struct es_video_attr *public_attr)
{
	int err;
	struct pollfd fd_set[1];
	struct v4l2_buffer v4l2_buf_param;

	/* request buffers */
    memset(&priv_attr->_v4l2_req_bufs, 0, sizeof(struct v4l2_requestbuffers));
    priv_attr->_v4l2_req_bufs.count = COUNT_OF_V4L2_REQ_BUFFER;
    priv_attr->_v4l2_req_bufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    priv_attr->_v4l2_req_bufs.memory = V4L2_MEMORY_MMAP;
    err = ioctl(fd, VIDIOC_REQBUFS, &priv_attr->_v4l2_req_bufs);
    if (err) 
    {
    	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to allocate buffers.\n" , __FUNCTION__);
        return ES_FAIL;        
    }
  	priv_attr->_vbuf_cnt_total = priv_attr->_v4l2_req_bufs.count;
	priv_attr->_max_vbuf_bytes = VIDEO_BUF_MAX_LEN(public_attr->resolution);
	if (priv_attr->_v4l2_cap.capabilities & V4L2_CAP_STREAMING)
    {
        /* map the buffers */
		int i = 0;
		int buf_total = priv_attr->_vbuf_cnt_total;
		priv_attr->_max_vbuf_bytes = VIDEO_BUF_MAX_LEN(public_attr->resolution);
		struct video_buf *tmp_vbuf = NULL;
        for (i = 0; i < buf_total; i++) 
        {
        	/*alloc video buffer array to index*/
			priv_attr->_vbuf_arr[i] = video_buf_create();
			tmp_vbuf = priv_attr->_vbuf_arr[i];
			if(NULL == tmp_vbuf)
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] no vbuf to alloc!\n" , __FUNCTION__);
        	    return ES_FAIL;
			}
			memset(tmp_vbuf, 0, sizeof(struct video_buf));
			memset(&v4l2_buf_param, 0, sizeof(struct v4l2_buffer));
        	v4l2_buf_param.index = i;
        	v4l2_buf_param.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	v4l2_buf_param.memory = V4L2_MEMORY_MMAP;
        	err = ioctl(fd, VIDIOC_QUERYBUF, &v4l2_buf_param);
        	if (err) 
            {
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] Unable to query buffer.\n" , __FUNCTION__);
        	    return ES_FAIL;
        	}
			tmp_vbuf->buf_bytes = v4l2_buf_param.length;
			tmp_vbuf->state = VBUF_FREE;
			tmp_vbuf->start_addr = MAP_FAILED;
			INIT_ES_LIST_HEAD(&tmp_vbuf->entry);
        	tmp_vbuf->start_addr = mmap(0 /* start anywhere */ ,
        			  tmp_vbuf->buf_bytes, PROT_READ, MAP_SHARED, 
        			  fd, v4l2_buf_param.m.offset);
        	if (MAP_FAILED == tmp_vbuf->start_addr) 
            {
            	tmp_vbuf->state = VBUF_UNKNOW;
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] Unable to map buffer.\n" , __FUNCTION__);
				return ES_FAIL;
        	}
			/* Queue the buffers. */
			es_common_delay(100);
			err = ioctl(fd, VIDIOC_QBUF, &v4l2_buf_param);
			if (err)
            {
            	ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] Unable to queue buffer.\n" , __FUNCTION__);
        	    return ES_FAIL;
        	}
        }
    }
    else if (priv_attr->_v4l2_cap.capabilities & V4L2_CAP_READWRITE)
    {
        /* read(fd, buf, size) */
		struct video_buf *tmp_vbuf = NULL;
        priv_attr->_vbuf_cnt_total  = 1;
        priv_attr->_max_vbuf_bytes = VIDEO_BUF_MAX_LEN(public_attr->resolution);
		priv_attr->_vbuf_arr[0] = malloc(sizeof(struct video_buf));
		tmp_vbuf = priv_attr->_vbuf_arr[0];
		if(NULL == tmp_vbuf)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] no vbuf to alloc!\n" , __FUNCTION__);
			return ES_FAIL;
		}
		tmp_vbuf->start_addr  = NULL;
		tmp_vbuf->buf_bytes = priv_attr->_max_vbuf_bytes;
		tmp_vbuf->state = VBUF_FREE;
		INIT_ES_LIST_HEAD(&tmp_vbuf->entry);
        tmp_vbuf->start_addr = malloc(priv_attr->_max_vbuf_bytes);
		if(NULL == tmp_vbuf->start_addr)
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] no vbuf to alloc!\n" , __FUNCTION__);
			return ES_FAIL;
		}
		priv_attr->_current_buf_idx = 0;
    }
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] do not support this buf model!\n" , __FUNCTION__);
		return ES_FAIL;
	}
	return ES_SUCCESS;
}

static inline es_error_t set_v4l2_es_media_frame(struct es_media_frame *out_frame, 
		struct video_buf *vbuf, 
		struct es_video_attr *public_attr)
{
	es_error_t ret;
		
	if((NULL == out_frame) || (NULL == vbuf) || (NULL == public_attr))
	{
		return ES_FAIL;
	}
	if(ES_PIX_FMT_UNKNOW != public_attr->pix_fomat)
	{
		ret = es_media_frame_buf_alloc(out_frame, vbuf->buf_bytes);
		if(ES_SUCCESS == ret)
		{
			out_frame->type = ES_PIXEL_FRAME;
			out_frame->attr.pix_frame.pix_fmt = public_attr->pix_fomat;
			out_frame->attr.pix_frame.bpp = public_attr->bpp;
			out_frame->attr.pix_frame.x_resolution= public_attr->resolution.x;
			out_frame->attr.pix_frame.y_resolution= public_attr->resolution.y;
			out_frame->buf_size = vbuf->buf_bytes;
			memcpy(out_frame->buf_start_addr, vbuf->start_addr, out_frame->buf_size);
		}
		else
		{
			out_frame->type = ES_UNKNOW_FRAME;
			out_frame->buf_size = 0;
			out_frame->buf_start_addr = NULL;
		}
		return ret;
	}
	else if(ES_VIDEO_COMPRESS_FMT_UNKNOW != public_attr->video_compress_fmt)
	{
		ret = es_media_frame_buf_alloc(out_frame, vbuf->buf_bytes);
		if(ES_SUCCESS == ret)
		{
			out_frame->type = ES_COMPRESS_VIDEO_FRAME;
			out_frame->attr.video_frame.compress_fmt  = public_attr->video_compress_fmt;
			out_frame->buf_size = vbuf->buf_bytes;
			memcpy(out_frame->buf_start_addr, vbuf->start_addr, out_frame->buf_size);
		}
		else
		{
			out_frame->type = ES_UNKNOW_FRAME;
			out_frame->buf_size = 0;
			out_frame->buf_start_addr = NULL;
		}
		return ret;
	}
	else
	{
		return ES_INVALID_PARAM;
	}
	return ES_SUCCESS;
}



static struct video_base v4l2_cmr = {
	.sub_class = ES_VIDEO_CLASS_CAMERA,
	.path_name = NULL,	
	.fd = -1,
	.priv = NULL,
	.video_open = v4l2_cmr_open,
	.video_close = v4l2_cmr_close,
	.video_start = v4l2_cmr_start,
	.video_stop = v4l2_cmr_stop,
	.video_get_attr = v4l2_cmr_get_attr,
	.video_set_attr = v4l2_cmr_set_attr,
	.video_get_ctrl = v4l2_cmr_get_ctrl,
	.video_set_ctrl = v4l2_cmr_set_ctrl,
	.video_send_frame = v4l2_cmr_send_frame,
	.video_recv_frame = v4l2_cmr_recv_frame,
};

/*******************************************************************************
* @function name: v4l2_cmr_open    
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
static es_error_t v4l2_cmr_open(const char *path, struct video_base *base)
{
	int i;
    es_error_t ret;
	int err;
	struct _v4l2_priv_attr *priv_attr = NULL;
	struct es_video_attr *public_attr = NULL;
	int fd;

	if((NULL == path) || (NULL == base))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
        return ES_INVALID_PARAM;
	}
	public_attr = &base->attr;
	priv_attr = malloc(sizeof(struct _v4l2_priv_attr));
	if(NULL == priv_attr)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] no memory now!\n" , __FUNCTION__);
		return ES_FAIL;
	}
	
    fd = open(path, O_RDWR);
    if (fd < 0)
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s]can not open %s\n" , __FUNCTION__, path);
        return ES_FAIL;
    }
   	err = ioctl(fd, VIDIOC_QUERYCAP, &priv_attr->_v4l2_cap);
    memset(&priv_attr->_v4l2_cap, 0, sizeof(struct v4l2_capability));
    err = ioctl(fd, VIDIOC_QUERYCAP, &priv_attr->_v4l2_cap);
    if (err) 
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Error opening device %s: unable to query device.\n" , __FUNCTION__, path);
    	goto ERR_EXIT;
    }
	public_attr->property = ES_VIDEO_PROPERTY_UNKNOW;
	public_attr->bpp = 0;
    if ((priv_attr->_v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
		public_attr->property |= ES_VIDEO_PROPERTY_CAPTURE;
    }
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] %s is not a video capture device\n" , __FUNCTION__, path);
        goto ERR_EXIT;
	}
	ret = set_v4l2_fmt_attr(fd, priv_attr, public_attr);
	if(ES_SUCCESS != ret)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] v4l2 set format error!\n" , __FUNCTION__);
        goto ERR_EXIT;
	}
	ret = set_v4l2_buf_attr(fd, priv_attr, public_attr);
	if(ES_SUCCESS != ret)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] v4l2 set buffer error!\n" , __FUNCTION__);
        goto ERR_EXIT;
	}
	base->sub_class = ES_VIDEO_CLASS_CAMERA;
	base->path_name = path;
	base->fd = fd;

	/*query all v4l2 ctrl*/

	public_attr->p_ctrl_list_head = NULL;
	INIT_ES_LIST_HEAD(&base->ctrl_list_head);
	public_attr->p_ctrl_list_head = &base->ctrl_list_head;
	struct v4l2_queryctrl qctrl;
	memset(&qctrl, 0, sizeof(struct v4l2_queryctrl));
	for(i = 0; i < V4L2_CAMERA_MAX_CTRL; i++)
	{
		qctrl.id = V4L2_CID_BASE + i;
		err = ioctl(fd, VIDIOC_QUERYCTRL, &qctrl);
		if(0 == err)
		{
			struct es_video_ctrl *tmp_ctrl = NULL;
			tmp_ctrl = malloc(sizeof(struct es_video_ctrl));
			if(NULL == tmp_ctrl)
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] no memory now!\n" , __FUNCTION__);
        		return ES_FAIL;
			}
			INIT_ES_LIST_HEAD(&tmp_ctrl->ctrl_entry);
			tmp_ctrl->ctrl_id = qctrl.id;
			memcpy(tmp_ctrl->name, qctrl.name, sizeof(unsigned char) * 32);
			tmp_ctrl->name[31] = '\0';
			tmp_ctrl->minimum = qctrl.minimum;
			tmp_ctrl->maximum = qctrl.maximum;
			tmp_ctrl->step = qctrl.step;
			tmp_ctrl->default_val = qctrl.default_value;
			tmp_ctrl->status = qctrl.flags;
			es_list_add_tail(&tmp_ctrl->ctrl_entry, public_attr->p_ctrl_list_head);
		}
	}
	// memcpy(&base->attr, &public_attr, sizeof(struct es_video_attr));
	INIT_ES_LIST_HEAD(&base->entry);
	INIT_ES_LIST_HEAD(&base->video_buf_head);
	base->priv = priv_attr;
    return ES_SUCCESS;
	
ERR_EXIT:    
    close(fd);
    return ES_FAIL;  
}

/*******************************************************************************
* @function name: v4l2_cmr_close    
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
static es_error_t v4l2_cmr_close(struct video_base *base)
{
	struct _v4l2_priv_attr *priv_attr = base->priv;
	struct video_buf *tmp_buf = NULL;
	int i = 0;
	int err;
	es_error_t ret = ES_SUCCESS;

	if((NULL == base) || (NULL == priv_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}
	if (priv_attr->_v4l2_cap.capabilities & V4L2_CAP_STREAMING)
	{
		for(i = 0; i < COUNT_OF_V4L2_REQ_BUFFER; i++)
		{
			tmp_buf = priv_attr->_vbuf_arr[i];
			if(NULL != tmp_buf)
			{
				err = munmap(tmp_buf->start_addr, tmp_buf->buf_bytes);
				if(err)
				{
					ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
					ES_PRINTF("[%s] munmap fail in close!\n" , __FUNCTION__);
				}
				tmp_buf->start_addr = NULL;
				tmp_buf->state = VBUF_UNKNOW;
				video_buf_destroy(tmp_buf);
				tmp_buf = NULL;
				priv_attr->_vbuf_arr[i] = NULL;
			}
		}
	}
	else if (priv_attr->_v4l2_cap.capabilities & V4L2_CAP_READWRITE)
    {
        /* read(fd, buf, size) */
		tmp_buf = priv_attr->_vbuf_arr[0];
		tmp_buf->state = VBUF_UNKNOW;
		video_buf_destroy(tmp_buf);
		priv_attr->_vbuf_arr[0] = NULL;
    }
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] do not support this buf model!\n" , __FUNCTION__);
	}
	
	ret = close(base->fd);
	free(base);
	base = NULL;
	return ret;
}
/*******************************************************************************
* @function name: v4l2_cmr_start    
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
static es_error_t v4l2_cmr_start(struct video_base *base)
{
	int video_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	struct _v4l2_priv_attr *priv_attr = base->priv;
	struct pollfd fd_set[1];
	struct v4l2_buffer v4l2_buf_param;
	struct video_buf *tmp_vbuf = NULL;
    int err;

	if((NULL == base) || (NULL == priv_attr))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}


    err = ioctl(base->fd, VIDIOC_STREAMON, &video_type);
    if (err) 
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to start capture.\n" , __FUNCTION__);
    	return ES_FAIL;
    }
	es_common_delay(100);
    fd_set[0].fd     = base->fd;
    fd_set[0].events = POLLIN;
    err = poll(fd_set, 1, -1);
    if (err <= 0)
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] poll error!\n" , __FUNCTION__);
        return ES_FAIL;
   	 }
    memset(&v4l2_buf_param, 0, sizeof(struct v4l2_buffer));
    v4l2_buf_param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf_param.memory = V4L2_MEMORY_MMAP;
    err = ioctl(base->fd, VIDIOC_DQBUF, &v4l2_buf_param);
    if (err < 0) 
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to dequeue buffer.\n" , __FUNCTION__);
    	return ES_FAIL;
    }
	/*get 1st video frame when start video, 
	this frame have not use, just for  setting
	priv_attr->_current_buf_idx to correct index*/	
   	priv_attr->_current_buf_idx = v4l2_buf_param.index;
	tmp_vbuf = priv_attr->_vbuf_arr[priv_attr->_current_buf_idx];
	tmp_vbuf->buf_bytes = v4l2_buf_param.length;
	tmp_vbuf->state = VBUF_USED;
    return ES_SUCCESS;
}
/*******************************************************************************
* @function name: v4l2_cmr_stop    
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
static es_error_t v4l2_cmr_stop(struct video_base *base)
{
	int video_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int err;

	if(NULL == base)
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

    err = ioctl(base->fd, VIDIOC_STREAMOFF, &video_type);
    if (err) 
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to start capture.\n" , __FUNCTION__);
    	return ES_FAIL;
    }
    return ES_SUCCESS;
}
/*******************************************************************************
* @function name: v4l2_cmr_send_frame    
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
static es_error_t v4l2_cmr_send_frame(struct video_base *base, struct es_media_frame *vframe)
{
	es_error_t ret = ES_SUCCESS;

	return ret;
}

/*******************************************************************************
* @function name: v4l2_cmr_recv_frame    
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
static es_error_t v4l2_cmr_recv_frame(struct video_base *base, struct es_media_frame *vframe)
{
	struct _v4l2_priv_attr *priv_attr = base->priv;
	struct es_video_attr *public_attr = &base->attr;
	struct pollfd fd_set[1];
	struct v4l2_buffer v4l2_buf_param;
	struct video_buf *tmp_vbuf = NULL;
    int err;
	es_error_t ret = ES_SUCCESS;
	struct es_media_frame *out_frame = NULL;

	if((NULL == base) 
		|| (NULL == priv_attr)
		|| (NULL == public_attr)
		|| (NULL == vframe))
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] input null pointer!\n" , __FUNCTION__);
		return ES_INVALID_PARAM;
	}

	if(priv_attr->_v4l2_cap.capabilities & V4L2_CAP_STREAMING)
	{
		/*release current buf first*/
		if((priv_attr->_current_buf_idx < 0) 
		|| (priv_attr->_current_buf_idx >= COUNT_OF_V4L2_REQ_BUFFER))
		{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] error buffer index.\n" , __FUNCTION__);
    		return ES_FAIL;
		}
		memset(&v4l2_buf_param, 0, sizeof(struct v4l2_buffer));
		tmp_vbuf = priv_attr->_vbuf_arr[priv_attr->_current_buf_idx];
		tmp_vbuf->buf_bytes = v4l2_buf_param.length;
		tmp_vbuf->state = VBUF_FREE;
		v4l2_buf_param.index  = priv_attr->_current_buf_idx;
		v4l2_buf_param.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2_buf_param.memory = V4L2_MEMORY_MMAP;
		err = ioctl(base->fd, VIDIOC_QBUF, &v4l2_buf_param);
		if (err) 
    	{
    		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Unable to queue buffer.\n" , __FUNCTION__);
	    	return ES_FAIL;
		}

		/*get date from buf*/
		fd_set[0].fd     = base->fd;
    	fd_set[0].events = POLLIN;
    	err = poll(fd_set, 1, -1);
    	if (err <= 0)
    	{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] poll error!\n" , __FUNCTION__);
        	return ES_FAIL;
   	 	}
    	memset(&v4l2_buf_param, 0, sizeof(struct v4l2_buffer));
    	v4l2_buf_param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	v4l2_buf_param.memory = V4L2_MEMORY_MMAP;
    	err = ioctl(base->fd, VIDIOC_DQBUF, &v4l2_buf_param);
    	if (err < 0) 
    	{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Unable to dequeue buffer.\n" , __FUNCTION__);
    		return ES_FAIL;
    	}
   		priv_attr->_current_buf_idx = v4l2_buf_param.index;
		tmp_vbuf = priv_attr->_vbuf_arr[priv_attr->_current_buf_idx];
		tmp_vbuf->buf_bytes = v4l2_buf_param.length;
		tmp_vbuf->state = VBUF_USED;
	
		ret = set_v4l2_es_media_frame(vframe, tmp_vbuf, public_attr);
		return ret;
	}
	else if(priv_attr->_v4l2_cap.capabilities & V4L2_CAP_READWRITE)
	{
		tmp_vbuf = priv_attr->_vbuf_arr[0];
		tmp_vbuf->buf_bytes = priv_attr->_max_vbuf_bytes;
		tmp_vbuf->state = VBUF_USED;
		err = read(base->fd, tmp_vbuf->start_addr, tmp_vbuf->buf_bytes);
		if (err <= 0)
    	{
        	return ES_FAIL;
    	}
		ret = set_v4l2_es_media_frame(vframe, tmp_vbuf, public_attr);
		return ret;

	}
	else
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] do not support this model!\n" , __FUNCTION__);
		vframe->type = ES_UNKNOW_FRAME;
		vframe->buf_size = 0;
		vframe->buf_start_addr = NULL;
		return ES_FAIL;
	}
}

/*******************************************************************************
* @function name: v4l2_cmr_get_ctrl    
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
static es_error_t v4l2_cmr_get_ctrl(struct video_base *base, struct es_video_ctrl_cmd *cmd)
{
	es_error_t ret = ES_SUCCESS;
	int err;

	struct es_video_ctrl *cur_ctrl = NULL;
	struct es_video_ctrl *tmp_ctrl = NULL;
	es_list_for_each_entry_safe(cur_ctrl, tmp_ctrl, &base->ctrl_list_head, ctrl_entry)
	{
		if(cur_ctrl->ctrl_id == cmd->ctrl_id)
		{
			struct v4l2_control tmp_cmd;
			tmp_cmd.id = cmd->ctrl_id;

			err = ioctl(base->fd, VIDIOC_G_CTRL, &tmp_cmd);
			if(0 == err)
			{
				cmd->ctrl_id = tmp_cmd.id;
				cmd->current_val = tmp_cmd.value;
				ret = ES_SUCCESS;
				return ret;
			}
			else
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] VIDIOC_G_CTRL fail!\n" , __FUNCTION__);
				ret = ES_FAIL;
				return ret;
			}
		}
			
	}
	ret = ES_FAIL;
	return ret;
}
/*******************************************************************************
* @function name: v4l2_cmr_set_ctrl    
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
static es_error_t v4l2_cmr_set_ctrl(struct video_base *base, struct es_video_ctrl_cmd *cmd)
{
	es_error_t ret = ES_SUCCESS;
	int err;

	struct es_video_ctrl *cur_ctrl = NULL;
	struct es_video_ctrl *tmp_ctrl = NULL;
	es_list_for_each_entry_safe(cur_ctrl, tmp_ctrl, &base->ctrl_list_head, ctrl_entry)
	{
		if(cur_ctrl->ctrl_id == cmd->ctrl_id)
		{
			struct v4l2_control tmp_cmd;
			if(cmd->current_val > cur_ctrl->maximum)
			{
				cmd->current_val = cur_ctrl->maximum;
			}
			if(cmd->current_val < cur_ctrl->minimum)
			{
				cmd->current_val = cur_ctrl->minimum;
			}
			tmp_cmd.id = cmd->ctrl_id;
			tmp_cmd.value = cmd->current_val;
			// ES_PRINTF("tmp_cmd.id: 0x%x,  tmp_cmd.value: %d\n", tmp_cmd.id, tmp_cmd.value);
			err = ioctl(base->fd, VIDIOC_S_CTRL, &tmp_cmd);
			if(0 == err)
			{
				ret = ES_SUCCESS;
				return ret;
			}
			else
			{
				ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
				ES_PRINTF("[%s] VIDIOC_S_CTRL fail!\n" , __FUNCTION__);
				ES_PRINTF("err %d\n",err);
				ret = ES_FAIL;
				return ret;
			}
		}
			
	}
	ret = ES_FAIL;
	return ret;
}

/*******************************************************************************
* @function name: es_init_v4l2_camera    
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
es_error_t es_init_v4l2_camera(void)
{
	es_error_t ret;
	
	ret = video_base_register(&v4l2_cmr);
	return ret;
}

static es_error_t v4l2_cmr_get_attr(struct video_base *base, struct es_video_attr *public_attr)
{
	es_error_t ret = ES_SUCCESS;
	int err, i;
	unsigned long fps;
	struct v4l2_format  v4l2_pix_fmt;
	struct v4l2_capability v4l2_cap;
	struct v4l2_streamparm streamparm;
	
	if((NULL == base) || (NULL == public_attr) )
	{
		ret = ES_FAIL;
		return ret;
	}

	err = ioctl(base->fd, VIDIOC_QUERYCAP, &v4l2_cap);
    if (err) 
	{
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Error opening device : unable to query device.\n" , __FUNCTION__);
    	goto ERR_EXIT;
    }
    if (v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
    {
		public_attr->property &= ES_VIDEO_PROPERTY_CAPTURE;
		public_attr->property |= ES_VIDEO_PROPERTY_CAPTURE;
    }
    /* enum and try format */
    memset(&v4l2_pix_fmt, 0, sizeof(struct v4l2_format));
    v4l2_pix_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    err = ioctl(base->fd, VIDIOC_G_FMT, &v4l2_pix_fmt); 
    if (err) 
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to set format\n" , __FUNCTION__);
        goto ERR_EXIT;        
    }
    public_attr->resolution.x = v4l2_pix_fmt.fmt.pix.width;
    public_attr->resolution.y = v4l2_pix_fmt.fmt.pix.height;
	if (is_support_pixel_fmt(v4l2_pix_fmt.fmt.pix.pixelformat))
	{
		public_attr->video_compress_fmt = ES_VIDEO_COMPRESS_FMT_UNKNOW;
		public_attr->pix_fomat = pixel_fmt_unify_trans(v4l2_pix_fmt.fmt.pix.pixelformat);
		public_attr->bpp = video_base_get_bpp(public_attr->pix_fomat);
	}
	else if(is_support_compress_video_fmt(v4l2_pix_fmt.fmt.pix.pixelformat))
	{
		public_attr->video_compress_fmt = compress_video_fmt_unify_trans(v4l2_pix_fmt.fmt.pix.pixelformat);
		public_attr->pix_fomat = ES_PIX_FMT_UNKNOW;
		public_attr->bpp = 0;
	}
	streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    err = ioctl(base->fd, VIDIOC_G_PARM, &streamparm);
	if (err) 
    {
		ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
		ES_PRINTF("[%s] Unable to VIDIOC_G_PARM\n" , __FUNCTION__);
        goto ERR_EXIT;        
    }
	
	fps = ceil(streamparm.parm.capture.timeperframe.denominator
			/ streamparm.parm.capture.timeperframe.numerator);
	public_attr->fps = fps;
	
	if(NULL == public_attr->p_ctrl_list_head)
	{
		struct v4l2_queryctrl qctrl;
		INIT_ES_LIST_HEAD(&base->ctrl_list_head);
		public_attr->p_ctrl_list_head = &base->ctrl_list_head;
		memset(&qctrl, 0, sizeof(struct v4l2_queryctrl));
		for(i = 0; i < V4L2_CAMERA_MAX_CTRL; i++)
		{
			qctrl.id = V4L2_CID_BASE + i;
			err = ioctl(base->fd, VIDIOC_QUERYCTRL, &qctrl);
			if(0 == err)
			{
				struct es_video_ctrl *tmp_ctrl = NULL;
				tmp_ctrl = malloc(sizeof(struct es_video_ctrl));
				if(NULL == tmp_ctrl)
				{
					ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
					ES_PRINTF("[%s] no memory now!\n" , __FUNCTION__);
        			return ES_FAIL;
				}
				INIT_ES_LIST_HEAD(&tmp_ctrl->ctrl_entry);
				tmp_ctrl->ctrl_id = qctrl.id;
				memcpy(tmp_ctrl->name, qctrl.name, sizeof(unsigned char) * 32);
				tmp_ctrl->name[31] = '\0';
				tmp_ctrl->minimum = qctrl.minimum;
				tmp_ctrl->maximum = qctrl.maximum;
				tmp_ctrl->step = qctrl.step;
				tmp_ctrl->default_val = qctrl.default_value;
				tmp_ctrl->status = qctrl.flags;
				es_list_add_tail(&tmp_ctrl->ctrl_entry, public_attr->p_ctrl_list_head);
			}
		}
	}
	ret = ES_SUCCESS;
	return ret;
ERR_EXIT:
	return ES_FAIL;

}
static es_error_t v4l2_cmr_set_attr(struct video_base *base, struct es_video_attr *public_attr)
{
	es_error_t ret = ES_SUCCESS;
	int err = 0;
	
	if((base->attr.pix_fomat != public_attr->pix_fomat)
	|| (base->attr.video_compress_fmt != public_attr->video_compress_fmt)
	|| (base->attr.resolution.x !=  public_attr->resolution.x)
	|| (base->attr.resolution.y !=  public_attr->resolution.y))
	{
		struct v4l2_format  v4l2_pix_fmt;
		memset(&v4l2_pix_fmt, 0, sizeof(struct v4l2_format));
    	v4l2_pix_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		err = ioctl(base->fd, VIDIOC_G_FMT, &v4l2_pix_fmt);
		if (err) 
    	{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Unable to VIDIOC_G_FMT\n" , __FUNCTION__);
        	goto ERR_EXIT;        
    	}
		v4l2_pix_fmt.fmt.pix.width = public_attr->resolution.x;
		v4l2_pix_fmt.fmt.pix.height = public_attr->resolution.y;
		if(ES_PIX_FMT_UNKNOW != public_attr->pix_fomat)
		{
		 	v4l2_pix_fmt.fmt.pix.pixelformat = es_video_to_v4l2_pixel_fmt(public_attr->pix_fomat);
		}
		else
		{
			v4l2_pix_fmt.fmt.pix.pixelformat = es_video_to_v4l2_compress_fmt(public_attr->pix_fomat);
		}
    	err = ioctl(base->fd, VIDIOC_S_FMT, &v4l2_pix_fmt); 
	}

	if(base->attr.fps != public_attr->fps)
	{
		struct v4l2_streamparm streamparm;

		streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	err = ioctl(base->fd, VIDIOC_G_PARM, &streamparm);
		if (err) 
    	{
			ES_PRINTF("file: %s, line: %d\n", __FILE__, __LINE__);
			ES_PRINTF("[%s] Unable to VIDIOC_G_PARM\n" , __FUNCTION__);
        	goto ERR_EXIT;        
    	}
		streamparm.parm.capture.timeperframe.denominator = public_attr->fps;
		streamparm.parm.capture.timeperframe.numerator = 1;
		err = ioctl(base->fd, VIDIOC_S_PARM, &streamparm);
	}

	return ret;
ERR_EXIT:
	return ES_FAIL;
}


