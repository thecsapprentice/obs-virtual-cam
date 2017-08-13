#include "share_queue.h"
#include "libavutil/pixfmt.h"
#include <cstring>
#include <iostream>

bool shared_queue_create(share_queue* q, int mode, int format, 
	int frame_width,int frame_height,int64_t frame_time,int qlength)
{
  std::cout << "shared_queue_create" << std::endl;
	if (!q)
		return false;

	if (!shared_queue_check(mode))
		return false;

	int size = 0;

	if (mode == ModeVideo){
	  std::cout << "Creating a Video Queue!" << std::endl;
		size = sizeof(queue_header) + (sizeof(frame_header) + VIDEO_SIZE)
			* qlength;
		//q->hwnd = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 
		//	0, size, MAPPING_NAMEV);
	}
	else{
	  std::cout << "Creating a Audio Queue!" << std::endl;
		size = sizeof(queue_header) + (sizeof(frame_header) + AUDIO_SIZE)
			* qlength;
		//q->hwnd = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		//	0, size, MAPPING_NAMEA);
	}

	std::cout << "Size is " << size << std::endl;
	std::cout << "Allocating sufficent space to hold " << qlength << " frames of data" << std::endl;
	//if (q->hwnd){
	  //q->header = (queue_header*)MapViewOfFile(q->hwnd, FILE_MAP_ALL_ACCESS, 0, 0, size);
	  q->header = (queue_header*)(new char[size]);
	  //}
	queue_header* q_head = q->header;

	
	if (q_head){

		q_head->header_size = sizeof(queue_header);
		q_head->element_header_size = sizeof(frame_header);

		if (mode == ModeVideo)
			q_head->element_size = sizeof(frame_header) + VIDEO_SIZE;
		else
			q_head->element_size = sizeof(frame_header) + AUDIO_SIZE;
		
		q_head->format = format;
		q_head->queue_length = qlength;
		q_head->write_index = 0;
		q_head->frame_width = frame_width;
		q_head->frame_height = frame_height;
		q_head->state = OutputStart;
		q_head->frame_time = frame_time;
		q_head->delay_frame = 5;
		q->mode = mode;	
		q->index = 0;
		q->in = 0;
		q->out = 0;
		pthread_mutex_init(&q->queue_lock, NULL);
		sem_init(&q->count_sem, 0, 0 );
		sem_init(&q->space_sem, 0, qlength);
		
	}
	
	return (q->header != nullptr);
}

bool shared_queue_open(share_queue* q, int mode)
{
     std::cout << "shared_queue_open" << std::endl;
     return false;
}

bool shared_queue_check(int mode)
{
  std::cout << "shared_queue_check" << std::endl;
  //HANDLE hwnd =nullptr;

	if (mode == ModeVideo){
	  //hwnd = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, MAPPING_NAMEV);
	  std::cout << "This is a Video Queue!" << std::endl;
	  return true;
	}
	else if (mode == ModeAudio){
	  //hwnd = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, MAPPING_NAMEA);
	  std::cout << "This is a Audio Queue!" << std::endl;
	  return true;
	}
	else
		return false;

	// if (hwnd){
	//   //CloseHandle(hwnd);
	// 	return false;
	// }
	// else
	// 	return true;
}

void shared_queue_close(share_queue* q)
{
    std::cout << "shared_queue_close" << std::endl;
	if (q && q->header){
	  //q->header->state = OutputStop;
		//UnmapViewOfFile(q->header);
		//CloseHandle(q->hwnd);
	  delete [] q->header;
		q->header = nullptr;
		//	q->hwnd = nullptr;
		q->index = -1;
		pthread_mutex_destroy(&q->queue_lock);
	}
}

bool shared_queue_set_delay(share_queue* q,int delay_video_frame)
{
    std::cout << "shared_queue_set_delay" << std::endl;
	if (!q || !q->header)
		return false;

	q->header->delay_frame = delay_video_frame;
	return true;
}

bool share_queue_init_index(share_queue* q)
{
    std::cout << "shared_queue_init_index" << std::endl;
	if (!q || !q->header)
		return false;


	if (q->mode == ModeVideo){ 
		q->index = q->header->write_index - q->header->delay_frame;
		if (q->index < 0)
			q->index += q->header->queue_length;

	}else if (q->mode == ModeAudio){
		int write_index = q->header->write_index;
		int delay_frame = q->header->delay_frame;
		int64_t frame_time = q->header->frame_time;
		int64_t last_ts = q->header->last_ts;
		uint64_t start_ts = last_ts - delay_frame*frame_time;

		int index = write_index;
		uint64_t frame_ts = 0;

		do{
			index--;
			if (index < 0)
				index += q->header->queue_length;

			int offset = q->header->header_size +
				(q->header->element_size)*index;
			uint8_t* buff = (uint8_t*)q->header + offset;
			frame_ts = ((frame_header*)buff)->timestamp;

		} while (frame_ts > start_ts && index != write_index);

		if (index == write_index){
			q->index = write_index-q->header->queue_length/2;
			if (q->index < 0)
				q->index += q->header->queue_length;
		}
		else
			q->index = index;
	}

	return true;
}

bool shared_queue_get_video_format(int* format ,int* width, 
	int* height, int64_t* avgtime)
{
    std::cout << "shared_queue_get_video_format" << std::endl;
	bool success =true;
	//HANDLE hwnd;
	//queue_header* header;
	
	//hwnd = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, MAPPING_NAMEV);
	//if (hwnd){
	  //header = (queue_header*)MapViewOfFile(hwnd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//}
	//else
	//	return false;

	//if (header){
	//	*format = header->format;
	//	*width = header->frame_width;
	//	*height = header->frame_height;
	//	*avgtime = (header->frame_time) / 100;	
	//	//UnmapViewOfFile(header);
	//}
	//else
	//	success = false;

	//CloseHandle(hwnd);
	return success;
}

bool shared_queue_get_video(share_queue* q, uint8_t** data, 
	uint32_t*linesize, uint64_t* timestamp)
{
  //std::cout << "shared_queue_get_video" << std::endl;
	if (!q || !q->header)
		return false;

	sem_wait( &q->count_sem );
	pthread_mutex_lock( &q->queue_lock );
	//{
	//int count;
	//  sem_getvalue( &q->count_sem, &count );
	//  std::cout << count << "  items remain in the ring queue." << std::endl;
	//}

	//std::cout << "Reader Locked." << std::endl;
	
	int offset = q->header->header_size +
		(q->header->element_size)*q->out;

	uint8_t* buff = (uint8_t*)q->header + offset;
	frame_header* head = (frame_header*)buff;
	buff += q->header->element_header_size;
	int height = q->header->frame_height;
	int planes = 0;

	switch (q->header->format) {
	case AV_PIX_FMT_NONE:
		return false;

	case AV_PIX_FMT_YUV420P:
		planes = 3;
		data[0] = buff;
		data[1] = data[0] + head->linesize[0] * height;
		data[2] = data[1] + head->linesize[1] * height / 2;
		break;

	case AV_PIX_FMT_NV12:
		planes = 2;
		data[0] = buff;
		data[1] = data[0] + head->linesize[0] * height;
		break;

	case AV_PIX_FMT_GRAY8:
	case AV_PIX_FMT_YUYV422:
	case AV_PIX_FMT_UYVY422:
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
		planes = 1;
		data[0] = buff;
		break;

	case AV_PIX_FMT_YUV444P:
		planes = 3;
		data[0] = buff;
		data[1] = data[0] + head->linesize[0] * height;
		data[2] = data[1] + head->linesize[1] * height;
		break;
	}

	for (int i = 0; i < planes; i++)
		linesize[i]=head->linesize[i] ;

	*timestamp = head->timestamp;

	q->out++;

	if (q->out >= q->header->queue_length) 
		q->out = 0; 

	//std::cout << "Reader UnLocked." << std::endl;
	pthread_mutex_unlock( &q->queue_lock );
	sem_post( &q->space_sem );	
	
	return true;
}
bool shared_queue_push_video(share_queue* q, uint32_t* linesize,
	uint32_t height,uint8_t** src,uint64_t timestamp)
{
  //std::cout << "shared_queue_push_video" << std::endl;
	if (!q || !q->header)
		return false;

	sem_wait( &q->space_sem );
	pthread_mutex_lock( &q->queue_lock );
	//std::cout << "Writer Locked." << std::endl;
	//{
	//  int space;
	//  sem_getvalue( &q->space_sem, &space );
	  //std::cout << space << " space remains in the ring queue." << std::endl;
	//}
	//std::cout << "Filling timestamp " << timestamp << " at " << q->in << " index with video" << std::endl;
	
	int offset = q->header->header_size +
		(q->header->element_size)*q->in;

	uint8_t* buff = (uint8_t*)q->header + offset;
	frame_header* head = (frame_header*)buff;
	uint8_t* data = (uint8_t*)buff + q->header->element_header_size;
	int planes = 0;

	switch (q->header->format) {
	case AV_PIX_FMT_NONE:
	  //std::cout << "Format: AV_PIX_FMT_NONE" << std::endl;
		return false;

	case AV_PIX_FMT_YUV420P:
	  //std::cout << "Format: AV_PIX_FMT_YUV420P" << std::endl;
		planes = 3;
		memcpy(data, src[0], linesize[0] * height);
		data += linesize[0] * height;
		memcpy(data, src[1], linesize[1] * height / 2);
		data += linesize[1] * height / 2;
		memcpy(data, src[2], linesize[2] * height / 2);	
		break;

	case AV_PIX_FMT_NV12:
	  //std::cout << "Format: AV_PIX_FMT_NV12" << std::endl;
		planes = 2;
		memcpy(data, src[0], linesize[0] * height);
		data += linesize[0] * height;
		memcpy(data, src[1], linesize[1] * height / 2);
		break;

	case AV_PIX_FMT_GRAY8:
	case AV_PIX_FMT_YUYV422:
	case AV_PIX_FMT_UYVY422:
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
	  //std::cout << "Format: AV_PIX_FMT_*" << std::endl;
		planes = 1;
		memcpy(data, src[0], linesize[0] * height);
		break;

	case AV_PIX_FMT_YUV444P:
	  //std::cout << "Format: AV_PIX_FMT_YUV444P" << std::endl;
		planes = 3;
		memcpy(data, src[0], linesize[0] * height);
		data += linesize[0] * height;
		memcpy(data, src[1], linesize[1] * height);
		data += linesize[1] * height;
		memcpy(data, src[2], linesize[2] * height);
		break;
	}

	for (int i = 0; i < planes;i++)
		head->linesize[i] = linesize[i];

	head->timestamp = timestamp;

	//q->header->write_index = q->index;

	q->in++;

	if (q->in >= q->header->queue_length){
	  //		q->header->state = OutputReady;
		q->in = 0;
	}

	//std::cout << "Writer UnLocked." << std::endl;
	pthread_mutex_unlock( &q->queue_lock );
	sem_post(&q->count_sem);
	
	return true;
}

bool shared_queue_get_audio(share_queue* q, uint8_t** data,
	uint32_t* size, uint64_t* timestamp)
{
  //std::cout << "shared_queue_get_audio" << std::endl;
	if (!q || !q->header)
		return false;

	if (q->index == q->header->write_index)
		return false;

	if (q->index < 0)
		share_queue_init_index(q);

	int offset = q->header->header_size +
		(q->header->element_size)*q->index;

	uint8_t* buff = (uint8_t*)q->header + offset;
	frame_header* head = (frame_header*)buff;
	buff += q->header->element_header_size;

	*data = buff;
	*size = head->linesize[0];
	*timestamp = head->timestamp;

	q->index++;

	if (q->index >= q->header->queue_length)
		q->index = 0;

	return true;
}

bool shared_queue_push_audio(share_queue* q, uint32_t size,
	uint8_t* src, uint64_t timestamp,uint64_t video_ts)
{
  //std::cout << "shared_queue_push_audio" << std::endl;
	if (!q || !q->header)
		return false;

	sem_wait( &q->space_sem );
	pthread_mutex_lock( &q->queue_lock );
	
	//std::cout << "Filling " << q->in << " index with audio" << std::endl;

	int offset = q->header->header_size +
		(q->header->element_size)*q->in;

	uint8_t* buff = (uint8_t*)q->header + offset;
	frame_header* head = (frame_header*)buff;
	uint8_t* data = (uint8_t*)buff + q->header->element_header_size;
	memcpy(data, src, size);
	head->linesize[0] = size;
	head->timestamp = timestamp;

	q->header->last_ts = video_ts;
	//q->header->write_index = q->index;
	q->in++;

	if (q->in >= q->header->queue_length){
		q->in = 0;
		//		q->header->state = OutputReady;
	}

	pthread_mutex_unlock(&q->queue_lock);
	sem_post(&q->count_sem);
	
	return true;
}
