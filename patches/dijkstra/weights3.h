//#include <limits.h>


#define INT_MAX 65535


#define N_VERTICES 179

#define INTTYPE short

INTTYPE** W;


int _W[N_VERTICES][N_VERTICES] = 
{

  {0,             5,       10, INT_MAX,      INT_MAX},
  {INT_MAX,       0,        3,       2,            9},
  {INT_MAX,       2,        0, INT_MAX,            1},
  {7,       INT_MAX,  INT_MAX,       0,            6},
  {INT_MAX, INT_MAX,  INT_MAX,       3,            0}

};
