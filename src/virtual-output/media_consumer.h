#ifndef __MEDIA_CONSUMER_H__
#define __MEDIA_CONSUMER_H__

#include "../queue/share_queue.h"
#include <pthread.h>

struct consumer_data {
  consumer_data( share_queue* v, share_queue* a, bool t) :
    video_source(v), audio_source(a), terminate(t) {}
  
  share_queue* video_source;
  share_queue* audio_source;
  bool terminate;
  const char* video_device;
  size_t video_imagesize;
};

class media_consumer {

 public:
  media_consumer(share_queue* video_source, share_queue* audio_source);
  ~media_consumer();

  bool initialize();
  
private:
  bool initialized;
  pthread_t consumer_tid;
  consumer_data data;
  
};


#endif
