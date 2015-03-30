#ifndef __BARRIER_H__
#define __BARRIER_H__
//
//#include <emmintrin.h>

#include <string.h>

#if 0
#define FENCE _mm_mfence()
#else
#define FENCE
#endif

typedef struct {
  int ctrn[128];
  int bctr[2]; 

  int count;
  int nthreads;
} user_barrier_t;


void inline user_barrier_init(user_barrier_t* b, int nthreads) {
  memset(b->ctrn, 0, 128*sizeof(b->ctrn[0]));
  b->bctr[0] = 0;
  b->bctr[1] = 0;

  b->count = 0;
  b->nthreads = nthreads;
}

void inline user_barrier(volatile user_barrier_t* b, int tid) {
  int id;

  b->ctrn[tid] ^=1;
  id = b->ctrn[tid];
  __sync_add_and_fetch(&b->bctr[id], 1);

  if (!tid) {
    while(b->bctr[id] < b->nthreads);
    b->bctr[id] = 0;
  }
  else
    while (b->bctr[id]);
}


#endif
