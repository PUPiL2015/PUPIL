#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

typedef struct {
  int64_t beat;
  int tag;
  int64_t timestamp;
  double global_rate;
  double window_rate;
  double instant_rate;
} heartbeat_record_t;

typedef struct {
  int pid;
  double min_heartrate;
  double max_heartrate;
  int64_t window_size;

  int64_t counter;
  int64_t buffer_depth;
  int64_t buffer_index;
  int64_t read_index;
  char    valid;


} HB_global_state_t;

typedef struct {
  int64_t first_timestamp;
  int64_t last_timestamp;

  int64_t* window;
  //int64_t window_size;
  int64_t current_index;
  
  int steady_state;
  double last_average_time;

  heartbeat_record_t* log;

  FILE* binary_file;
  FILE* text_file;
  char filename[256];
  pthread_mutex_t mutex;

  HB_global_state_t* state;

} heartbeat_t;

int heartbeat_init(heartbeat_t * hb, 
		   double min_target, 
		   double max_target, 
		   int64_t window_size, 
		   int64_t buffer_depth,
		   char* log_name);

void heartbeat_finish(heartbeat_t * hb);

void hb_get_current(heartbeat_t volatile * hb, 
		    heartbeat_record_t volatile * record);

int hb_get_history(heartbeat_t volatile * hb,
		   heartbeat_record_t volatile * record,
		   int n);

double hb_get_global_rate(heartbeat_t volatile * hb);

double hb_get_windowed_rate(heartbeat_t volatile * hb);

double hb_get_min_rate(heartbeat_t volatile * hb);

double hb_get_max_rate(heartbeat_t volatile * hb);

int64_t hb_get_window_size(heartbeat_t volatile * hb);

int64_t heartbeat( heartbeat_t* hb, 
		   int tag );


#endif 
