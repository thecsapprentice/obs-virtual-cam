#include "media_consumer.h"
#include <iostream>

#include "libavutil/pixfmt.h"
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#include <errno.h>

#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#define BASE_ALIGNMENT 32
#define ALIGN_SIZE(size, align) \
	size = (((size)+(align-1)) & (~(align-1)))

void print_format(struct v4l2_format*vid_format) {
  printf("	vid_format->type                =%d\n",	vid_format->type );
  printf("	vid_format->fmt.pix.width       =%d\n",	vid_format->fmt.pix.width );
  printf("	vid_format->fmt.pix.height      =%d\n",	vid_format->fmt.pix.height );
  printf("	vid_format->fmt.pix.pixelformat =%d\n",	vid_format->fmt.pix.pixelformat);
  printf("	vid_format->fmt.pix.field       =%d\n",	vid_format->fmt.pix.field );
  printf("	vid_format->fmt.pix.colorspace  =%d\n",	vid_format->fmt.pix.colorspace );
  printf("      sizeimage                       =%d\n", vid_format->fmt.pix.sizeimage );
  printf("      bytesperline                    =%d\n", vid_format->fmt.pix.bytesperline );
}

int format_properties(const unsigned int format,
		      const unsigned int width,
		      const unsigned int height,
		      size_t*linewidth,
		      size_t*framewidth) {
  size_t lw, fw;
  switch(format) {
  case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YVU420:
    lw = width; /* ??? */
    fw = ROUND_UP_4 (width) * ROUND_UP_2 (height);
    fw += 2 * ((ROUND_UP_8 (width) / 2) * (ROUND_UP_2 (height) / 2));
    break;
  case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_Y41P: case V4L2_PIX_FMT_YUYV: case V4L2_PIX_FMT_YVYU:
    lw = (ROUND_UP_2 (width) * 2);
    fw = lw * height;
    break;
  default:
    return 0;
  }
  
  if(linewidth)*linewidth=lw;
  if(framewidth)*framewidth=fw;
  
  return 1;
}

bool set_video_format( int fdwr, consumer_data* data ){

  
  struct v4l2_capability vid_caps;
  struct v4l2_format vid_format;
  int ret_code;
  
  ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
  if(ret_code == -1){
    std::cout << "Could not query video device for capabilities." << std::endl;
    return false;
  }
  
  memset(&vid_format, 0, sizeof(vid_format));

  vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  
  ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
  print_format(&vid_format);

  vid_format.fmt.pix.width = data->video_source->header->frame_width;
  vid_format.fmt.pix.height = data->video_source->header->frame_height;
  //vid_format.fmt.pix_mp.field = V4L2_FIELD_NONE;
  //vid_format.fmt.pix_mp.colorspace = V4L2_COLORSPACE_SRGB;

  size_t width = vid_format.fmt.pix.width;
  size_t height = vid_format.fmt.pix.height;
  size_t size = 0;
  int alignment = BASE_ALIGNMENT;
  
  switch (data->video_source->header->format) {
  case AV_PIX_FMT_NONE:
    return false;
  case AV_PIX_FMT_YUV420P: 
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    size = width * height;
    ALIGN_SIZE( size, alignment );
    size+= (width/2) * (height/2);
    ALIGN_SIZE( size, alignment );
    size+= (width/2) * (height/2);
    ALIGN_SIZE( size, alignment );
    vid_format.fmt.pix.sizeimage = size;
    vid_format.fmt.pix.bytesperline = width;
    break;
  case AV_PIX_FMT_NV12:
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    size = width * height;
    ALIGN_SIZE( size, alignment );
    size += (width/2) * (height/2) * 2;
    ALIGN_SIZE( size, alignment );
    vid_format.fmt.pix.sizeimage = size;
    vid_format.fmt.pix.bytesperline = width;
    break;
  case AV_PIX_FMT_GRAY8:
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
    size = width * height;
    ALIGN_SIZE( size, alignment );
    vid_format.fmt.pix.sizeimage = size;
    vid_format.fmt.pix.bytesperline = width;
    break;
  case AV_PIX_FMT_YUYV422:
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    size = width * height * 2;
    ALIGN_SIZE( size, alignment );
    vid_format.fmt.pix.sizeimage = size;
    vid_format.fmt.pix.bytesperline = width*2;
    break;
  case AV_PIX_FMT_UYVY422:
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
    size = width * height * 2;
    ALIGN_SIZE( size, alignment );
    vid_format.fmt.pix.sizeimage = size;
    vid_format.fmt.pix.bytesperline = width * 2;
    break;
  case AV_PIX_FMT_RGBA:
    return false;
    break;
  case AV_PIX_FMT_BGRA:
    return false;
    break;
  case AV_PIX_FMT_YUV444P:
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV444;
    size = width * height * 3;
    ALIGN_SIZE( size, alignment );
    vid_format.fmt.pix.sizeimage = size;
    vid_format.fmt.pix.bytesperline = width;
  }
  data->video_imagesize = size;
  
  print_format(&vid_format);
  ret_code = ioctl(fdwr, VIDIOC_S_FMT, &vid_format);
  
  if(ret_code == -1){
    perror("Could not set video frame format.");
    return false;
  }
  
  print_format(&vid_format);

  return true;
}

void* consumer_routine( void* arg ){
  
  consumer_data* data = (consumer_data*)(arg);

  uint8_t* image_planes[8];
  uint32_t linesize[8];
  uint64_t timestamp;
  int fdwr = 0;
  fdwr = open(data->video_device, O_RDWR);
  memset( linesize, 0, sizeof(uint32_t)*8 );

  set_video_format( fdwr, data );
  
  while( !data->terminate ){
    //std::cout << "Consumer running" << std::endl;

    shared_queue_get_video( data->video_source,
    			    image_planes, linesize, &timestamp );
    
    //std::cout << "Consumed frame at timestamp: " << timestamp << std::endl;
    
    size_t wbytes = write( fdwr, image_planes[0], data->video_imagesize );
    //std::cout << "Wrote " << wbytes << " of " <<  data->video_imagesize << " bytes to video device." << std::endl;
  }

  close(fdwr);
  std::cout << "Consumer Done." << std::endl;
  pthread_exit(NULL);
}


media_consumer::media_consumer(share_queue* video_source, share_queue* audio_source)
  : data(video_source,  audio_source, false), initialized( false )
{
  
  
}

media_consumer::~media_consumer()
{
  data.terminate = true;
  
  if( initialized ){
    void* status;
    pthread_join( consumer_tid, &status );
    
  }
}


bool 
media_consumer::initialize()
{
  if( initialized )
    return true;
  
  data.video_device="/dev/video1";
  
  int fdwr = 0;
  int ret_code = 0;
  
  int i;

# define FRAME_WIDTH  640
# define FRAME_HEIGHT 480
# define FRAME_FORMAT V4L2_PIX_FMT_YVU420
  size_t framesize;
  size_t linewidth;
  
  std::cout << "Using output device: " << data.video_device << std::endl;
  
  fdwr = open(data.video_device, O_RDWR);
  if(fdwr < 0){
    std::cout << "Could not open video device " << data.video_device << std::endl;
    return false;
  }

  set_video_format( fdwr, &data );

  /*  if(!format_properties(vid_format.fmt.pix_mp.pixelformat,
                        vid_format.fmt.pix_mp.width, vid_format.fmt.pix_mp.height,
                        &linewidth,
                        &framesize)) {
    printf("unable to guess correct settings for format '%d'\n", FRAME_FORMAT);
    }*/

  close(fdwr);
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc = pthread_create( &consumer_tid, &attr, consumer_routine, (void*)&data );
  
  if( !rc )
    initialized = true;

  pthread_attr_destroy(&attr);
  return initialized;
}


    
