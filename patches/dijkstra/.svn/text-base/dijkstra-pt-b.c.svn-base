#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include "barrier.h"
#include "heartbeat-accuracy-power.h"
heartbeat_t heart;
#include "weights3.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))


void init_weights(int n) {
  int i,j;

  for(i = 0; i < n; i++) {
    for(j = 0; j < n; j++) {
      double v = drand48();
      
      if(n != 5) {
	if(v < 0.8)
	  W[i][j] = (int) (v*100) + 1;
	else
	  W[i][j] = 10000;
      
	if(i == j)
	  W[i][j] = 0;
      }

    }
  }

}

int initialize_single_source(int** W, int* d, int* q, int source);
int get_min(int* Q, int* D, int n);
int relax(int u, int i, volatile int* D, int** W);

int local_min_buffer[100];
int global_min_buffer;


int get_local_min(volatile int* Q, volatile int* D, int start, int stop);

typedef struct {

  int* local_min;
  int* global_min;
  int* Q;
  int* D;
  int* d_count;
  int  tid;
  int  P;
  int  N;
  user_barrier_t* ubarrier;
  pthread_barrier_t* barrier1;
  pthread_barrier_t* barrier2;
} thread_arg_t;

thread_arg_t thread_arg[100];
pthread_t   thread_handle[100];

void* do_work(void* args) {
  volatile thread_arg_t* arg = (thread_arg_t*) args;

  volatile int* count      = arg->d_count;
  volatile int* global_min = arg->global_min;
  volatile int* local_min  = arg->local_min;
  int tid     = arg->tid;
  int  P      = arg->P;
  volatile int* Q      = arg->Q;
  int* D      = arg->D;
  const int N = arg->N;
  int i;
  int local_count = N;

  int i_start =  tid    * arg->N / (arg->P);
  int i_stop  = (tid+1) * arg->N / (arg->P);

  while(local_count != 0) {
  if(N <= 20) {
    for(i = 0; i < N; i++) {
      //printf("%12d, %12d ", D[i], Q[i]);
    }
    //printf("\n");
  }
    local_min[tid] = get_local_min(Q, D, i_start, i_stop);
    //printf("Thread %d: Waiting at barrier\n", tid);
    //printf("local min = %d, start = %d, stop = %d\n", local_min[tid], i_start, i_stop);
    user_barrier(arg->ubarrier, tid);
    
    if(tid == 0) {
      int min = D[local_min[tid]];
      int min_index = local_min[tid];
      for(i = 0; i < P; i++) {
	if(D[local_min[i]] < min && Q[local_min[i]]) {
	  min = D[local_min[i]];
	  min_index = local_min[i];
	}
      }
      *count = *count - 1;
      *global_min = min_index;
      Q[*global_min] = 0;
    }
    //printf("Thread %d: Waiting at barrier\n", tid);
    pthread_barrier_wait(arg->barrier1);
    user_barrier(arg->ubarrier, tid);
     //#pragma omp barrier
    int u = *global_min;
    //printf("Thread %d: Count is %d, global_min is %d\n", tid, *count, u);
      
    for(i = i_start; i < i_stop; i++) {
      relax(u, i, D, W);
    }
    //for(i = 0; i < N; i++) {
    //  printf("%12d ", D[i]);
    //}
    //printf("\n");
    user_barrier(arg->ubarrier, tid);
    pthread_barrier_wait(arg->barrier1);
    local_count--;
    //printf("Thread %d: Count is %d\n", tid, *count);
  }

}


int main(int argc, char** argv) {
  int i,j;
  const int N = N_VERTICES;
  double wtime;
  double wtime1;
  double wtime2;
  int D[N_VERTICES+1];
  int Q[N_VERTICES];
  int d_count = N;
  const int P = atoi(argv[1]);
  pthread_barrier_t barrier1;
  pthread_barrier_t barrier2;
  user_barrier_t ubarrier;
   int ii_tmp = 0;
        heartbeat_init(&heart,
                   0.0, 100.0,
                   0, 100,
                   0, 1000,
                   1, 1, "heartbeat.log");
for(ii_tmp = 0; ii_tmp < 10; ii_tmp++){
  W = (int**) malloc(N_VERTICES*sizeof(int*));
  for(i = 0; i < N; i++) {
    W[i] = (int*) malloc(N_VERTICES*sizeof(int));
    for(j = 0; j < N; j++) {
      if( N_VERTICES != 5) {
	double v = drand48();
	
	if(v < 0.8)
	  W[i][j] = (int) (v*100) + 1;
	else
	  W[i][j] = 10000;
	
	if(i == j)
	  W[i][j] = 0;
      }
      else {
	W[i][j] = _W[i][j];
	//printf("%d\n", W[i][j]);
      }
    }
  }


  pthread_barrier_init(&barrier1,NULL,P);
  pthread_barrier_init(&barrier2,NULL,P);

  user_barrier_init(&ubarrier, P);

  initialize_single_source(W,D,Q,0);

  if(N <= 20) {
    for(i = 0; i < N; i++) {
      printf("%12d ", D[i]);
    }
    printf("\n");
  }

  wtime1 = omp_get_wtime ( );
  for(j = 0; j < P; j++) {
    thread_arg[j].local_min  = local_min_buffer;
    thread_arg[j].global_min = &global_min_buffer;
    thread_arg[j].Q          = Q;
    thread_arg[j].D          = D;
    thread_arg[j].d_count    = &d_count;
    thread_arg[j].tid        = j;
    thread_arg[j].P          = P;
    thread_arg[j].N          = N;
    thread_arg[j].ubarrier   = &ubarrier;
    thread_arg[j].barrier1   = &barrier1;
    thread_arg[j].barrier2   = &barrier2;
	
    pthread_create(thread_handle+j,
		   NULL,
		   do_work,
		   (void*)&thread_arg[j]);
  }
  for(j = 0; j < P; j++) {
    //printf("Joining %d\n", j);
    pthread_join(thread_handle[j],NULL);
  }
  wtime2 = omp_get_wtime ( );
  wtime = wtime2 - wtime1;

  if(N <= 20) {
    for(i = 0; i < N; i++) {
      printf("%12d ", D[i]);
    }
    printf("\n");
  }
  printf ( "Time     = %e\n", wtime );
  printf ("N_VERTICES = %d\n",N_VERTICES);


        heartbeat(&heart, ii_tmp, 1);
        heartbeat_record_t record;
        hb_get_current(&heart, &record);

}
       heartbeat_finish(&heart);  
return 0;
}

int initialize_single_source(int** W, 
			     int*  d, 
			     int*  q, 
			     int   source) {
  int i;

  for(i = 0; i < N_VERTICES+1; i++) {
    d[i] = INT_MAX;
    q[i] = 1;
  }

  d[source] = 0;

  return 0;
}


int get_min(int* Q, int* D, int n) {

  int i;

  int min = INT_MAX;

  for(i = 0; i < n; i++) {
    if(D[i] < min && Q[i]) {
      min = i;
    }
  }
  return min;

}

int get_local_min(volatile int* Q, volatile int* D, int start, int stop) {

  int i;

  int min = INT_MAX;
  int min_index = N_VERTICES;

  for(i = start; i < stop; i++) {
    if(D[i] < min && Q[i]) {
      min = D[i];
      min_index = i;
    }
  }
  return min_index;

}

#include <math.h>

int relax(int u, int i, volatile int* D, int** W) {
  int j;
  if(D[i] > D[u] + W[u][i]) {
    //for(j = 0; j < 1000; j++) 
    D[i] = D[u] + W[u][i];
  }

  return 0;
}
