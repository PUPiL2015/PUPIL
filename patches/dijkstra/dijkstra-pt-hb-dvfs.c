#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
//#include <omp.h>
#include "barrier.h"
#include <heartbeat-accuracy-power.h>
#define _GNU_SOURCE
#include <sched.h>
#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

heartbeat_t heart;
  int window_size;
float power_target;

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

int initialize_single_source(INTTYPE** W, int* d, int* q, int source, int N);
int get_min(int* Q, int* D, int n);
int relax(int u, int i, volatile int* D, INTTYPE** W);

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

int hb_counter = 0;
//---------------------------------------------------
// Control stuff

int current_action = -1;
int h1, h2, h3, h4;
int max_actions = 0;

typedef struct {
  int beat;
  double rate;
  int state;
} controller_log_entry_t;


controller_log_entry_t controller_log[1000];


#define FAST 1
#define SLOW 1

#if FAST 
double p1 = 0.0;
double p2 = 0.0;
double z1 = 0.0;
#elif SLOW
const double p1 = 0.01;
const double p2 = 0.1;
const double z1 = 0.05;
#else
const double p1 = -0.5;
const double p2 =  0.0;
const double z1 =  0.0;
#endif

const double mu = 1.0;
//double last_speedup = 1.0;
#define STARTING_POWERUP 7.4 
#define STARTING_SPEEDUP 3
double last_powerup = STARTING_POWERUP;
double last_speedup = STARTING_SPEEDUP;

double act_acc  = 0.0;                 // and the idled application

double power_u         = STARTING_POWERUP;                   // control variable
double power_uo        = STARTING_POWERUP;                   // old value for the control variable
double power_uoo       = STARTING_POWERUP;
double power_e         = 100000;
double power_eo        = 100000;

double speed_u         = STARTING_SPEEDUP;                   // control variable
double speed_uo        = STARTING_SPEEDUP;                   // old value for the control variable
double speed_uoo       = STARTING_SPEEDUP;
double speed_e         = 100000;
double speed_eo        = 100000;

int state_1 = 0;
int state_2 = 0;
int power_tdo_state_1 = 0;
int power_tdo_state_2 = 0;
int speed_tdo_state_1 = 0;
int speed_tdo_state_2 = 0;
int current_beat = 0;
int npids = 0;
pid_t pids[128]; 
int apps[1024];

volatile cpu_set_t cpumask[1];
volatile int freqsetting[1] = {3201000};

#if 0
int get_all_subpids(pid_t pid, pid_t* pids) {
  int   n=0;
  FILE  *cret;
  int   stret;
  char  pid_string[128];
  char  cretpids[100];	
  sprintf(cretpids,"ps -eLf | awk '(/%d/) && (!/awk/) {print $4}'",pid);
  cret = popen(cretpids, "r");

  if (cret == NULL) { 
    printf("Error in popen\n"); 
    exit(-1); 
  }

  while (fgets(pid_string, 128, cret) != NULL)  { 
      pids[n] = atoi(pid_string); 
      printf(" from ps - %d\n", pids[n]);
      n++; 
    }

  stret = pclose(cret);

  if (stret == -1) { 
    printf("Error in pclose\n"); 
    exit(-1); 
  }

  return n;
}
#endif

typedef int state_id_t;

//#define NSTATES 17
#define NSYSSTATES 53
//efine NSYSSTATES 12
#define NAPPSTATES 52

typedef struct {
  int id;

  double speedup;
  double cost;
} control_state_t;

typedef struct {

  int id;

  double freq;
  int cores; 
} power_state_t;

typedef struct {
  int id;

  int ref;
  int range;
  int subme;
} speed_state_t;

typedef struct {
  int id;

  int ref;
  int range;
  int subme;
} accuracy_state_t;


control_state_t power_controller_states[] = {
#if NSYSSTATES == 53
{	0	,	1	,	1	},
{	1	,	1.163271732	,	1.065459386	},
{	2	,	1.326450802	,	1.152370031	},
{	3	,	2.000779119	,	1.158258946	},
{	4	,	2.342362224	,	1.255978706	},
{	5	,	2.998628978	,	1.299315462	},
{	6	,	3.497754833	,	1.439517358	},
{	7	,	3.998042748	,	1.444595407	},
{	8	,	4.008467244	,	1.589721291	},
{	9	,	4.971273774	,	1.592954925	},
{	10	,	5.930045717	,	1.726970261	},
{	11	,	6.112010628	,	1.752157068	},
{	12	,	6.294323495	,	1.772866618	},
{	13	,	6.540468322	,	1.816738742	},
{	14	,	6.830129396	,	1.847993334	},
{	15	,	7.014754091	,	1.875424987	},
{	16	,	7.22679923	,	1.895050481	},
{	17	,	7.363138412	,	2.029663413	},
{	18	,	7.673919847	,	2.073628968	},
{	19	,	7.872333244	,	2.092926031	},
{	20	,	8.135646106	,	2.131456092	},
{	21	,	8.391979993	,	2.167973466	},
{	22	,	8.404035533	,	2.301570847	},
{	23	,	8.711506659	,	2.349388177	},
{	24	,	8.888993528	,	2.362690163	},
{	25	,	9.282032517	,	2.425433104	},
{	26	,	9.57516275	,	2.454561209	},
{	27	,	9.739863181	,	2.636924624	},
{	28	,	10.0788924	,	2.687586586	},
{	29	,	10.4058207	,	2.74523928	},
{	30	,	10.79348906	,	2.769645477	},
{	31	,	10.80531768	,	2.974609988	},
{	32	,	11.15136748	,	3.019535793	},
{	33	,	11.51035878	,	3.096745832	},
{	34	,	11.91013564	,	3.121659746	},
{	35	,	12.18132289	,	3.376517983	},
{	36	,	13.08318039	,	3.442629909	},
{	37	,	13.29497213	,	3.812376705	},
{	38	,	13.69964495	,	3.880726313	},
{	39	,	14.18314966	,	3.955659357	},
{	40	,	14.38812127	,	4.260517126	},
{	41	,	14.81921322	,	4.346649056	},
{	42	,	15.35247753	,	4.420098325	},
{	43	,	15.36429858	,	4.744361745	},
{	44	,	15.84191262	,	4.828515862	},
{	45	,	16.51979142	,	4.96835753	},
{	46	,	16.97220796	,	5.334766603	},
{	47	,	17.67476894	,	5.486239027	},
{	48	,	18.11240302	,	5.917762179	},
{	49	,	18.7258797	,	6.043033844	},
{	50	,	18.99855523	,	7.015605922	},
{	51	,	19.68798937	,	7.176472013	},
{	52	,	20.52524998	,	7.349292258	}
#else
{	0	,	1	,	1	},
{	1	,	1.163271732	,	1.065459386	},
{	2	,	1.326450802	,	1.152370031	},
{	3	,	1.488810095	,	1.24329113	},
{	4	,	1.650068787	,	1.353627355	},
{	5	,	1.80753684	,	1.468675067	},
{	6	,	1.967211766	,	1.584900326	},
{	7	,	2.125857007	,	1.733380748	},
{	8	,	2.280853438	,	1.860046486	},
{	9	,	2.437647326	,	2.042786943	},
{	10	,	2.591623527	,	2.253330233	},
{	11	,	3.080682296	,	3.09230493	}
#endif
};

control_state_t speed_controller_states[] = {
{	0	,	1	,	0	},
#if 1
{	1	,	1.077	,	0.11	},
{	2	,	1.0797	,	0.1175	},
{	3	,	1.1032	,	0.14	},
{	4	,	1.1902	,	0.16	},
{	5	,	1.2182	,	0.21	},
{	6	,	1.2187	,	0.23	},
{	7	,	1.2415	,	0.2425	},
{	8	,	1.3105	,	0.2775	},
{	9	,	1.333	,	0.3025	},
{	10	,	1.404	,	0.4325	},
{	11	,	1.4055	,	0.4325	},
{	12	,	1.4257	,	0.4725	},
{	13	,	1.52	,	0.535	},
{	14	,	1.5405	,	0.6475	},
{	15	,	1.5407	,	0.6475	},
{	16	,	1.556	,	0.7475	},
{	17	,	1.6357	,	0.8425	},
{	18	,	1.653	,	0.905	},
{	19	,	1.674	,	0.9575	},
{	20	,	1.676	,	0.9575	},
{	21	,	1.8322	,	1.14	},
{	22	,	1.8402	,	1.1475	},
{	23	,	1.915	,	1.32	},
{	24	,	1.936	,	1.33	},
{	25	,	1.9557	,	1.38	},
{	26	,	1.9565	,	1.38	},
{	27	,	2.0372	,	1.595	},
{	28	,	2.0842	,	2.13	},
//{	29	,	2.1747	,	2.4025	},
//{	30	,	2.192	,	2.435	},
{	31	,	2.25 /*2.2545*/	,	2.485	},
{	32	,	2.2665	,	2.4975	},
{	33	,	2.3835	,	2.68	},
{	34	,	2.416	,	2.6875	},
{	35	,	2.4507	,	2.7375	},
{	36	,	2.5765	,	2.9575	},
{	37	,	2.589	,	3.0125	},
{	38	,	2.6955	,	3.5325	},
{	39	,	2.701	,	3.545	},
{	40	,	2.7665	,	4.82	},
{	41	,	2.827	,	4.845	},
{	42	,	2.8807	,	4.85	},
{	43	,	3.1925	,	5.18	},
{	44	,	3.2432	,	5.2775	},
{	45	,	3.3797	,	5.4325	},
{	46	,	3.3822	,	5.5775	},
{	47	,	3.4307	,	5.6225	},
{	48	,	3.457	,	5.705	},
{	49	,	3.7525	,	5.7325	},
{	50	,	3.7615	,	5.7325	},
{	51	,	4.1185	,	6.2425	},
{	52	,	4.2537	,	6.255	},
#endif
{	53	,	7/*4.2615*/	,	6.295	}
};

speed_state_t speed_states[] = {
{	0	,	5,16,7	},
#if 1
{	1	,	5,15,7	},
{	2	,	5,14,7	},
{	3	,	5,12,7	},
{	4	,	5,10,7	},
{	5	,	4,15,7	},
{	6	,	4,14,7	},
{	7	,	4,12,7	},
{	8	,	4,11,7	},
{	9	,	4,10,7	},
{	10	,	3,15,7	},
{	11	,	3,14,7	},
{	12	,	3,12,7	},
{	13	,	3,10,7	},
{	14	,	3,9,7	},
{	15	,	3,8,7	},
{	16	,	2,12,7	},
{	17	,	3,6,7	},
{	18	,	2,10,7	},
{	19	,	2,9,7	},
{	20	,	2,8,7	},
{	21	,	1,13,7	},
{	22	,	1,12,7	},
{	23	,	1,11,7	},
{	24	,	1,10,7	},
{	25	,	1,9,7	},
{	26	,	1,8,7	},
{	27	,	1,6,7	},
{	28	,	1,4,7	},
//{	29	,	2,7,6	},
//{	30	,	2,6,6	},
{	31	,	1,13,6	},
{	32	,	1,12,6	},
{	33	,	1,11,6	},
{	34	,	1,10,6	},
{	35	,	1,9,6	},
{	36	,	1,7,6	},
{	37	,	1,6,6	},
{	38	,	1,5,6	},
{	39	,	1,4,6	},
{	40	,	3,11,1	},
{	41	,	3,10,1	},
{	42	,	3,9,1	},
{	43	,	3,7,1	},
{	44	,	3,6,1	},
{	45	,	3,5,1	},
{	46	,	3,4,1	},
{	47	,	1,12,1	},
{	48	,	2,7,1	},
{	49	,	1,9,1	},
{	50	,	1,8,1	},
{	51	,	1,6,1	},
{	52	,	1,5,1	},
#endif
{	53	,	1,4,1	}
};

power_state_t power_states[] = {
#if NSYSSTATES == 53
{	0	,	1.2	,	1	},
{	1	,	1.4	,	1	},
{	2	,	1.6	,	1	},
{	3	,	1.2	,	2	},
{	4	,	1.4	,	2	},
{	5	,	1.2	,	3	},
{	6	,	1.4	,	3	},
{	7	,	1.2	,	4	},
{	8	,	1.6	,	3	},
{	9	,	1.2	,	5	},
{	10	,	1.2	,	6	},
{	11	,	1.2	,	7	},
{	12	,	1.2	,	8	},
{	13	,	1.2	,	9	},
{	14	,	1.2	,	10	},
{	15	,	1.2	,	11	},
{	16	,	1.2	,	12	},
{	17	,	1.4	,	8	},
{	18	,	1.4	,	9	},
{	19	,	1.4	,	10	},
{	20	,	1.4	,	11	},
{	21	,	1.4	,	12	},
{	22	,	1.6	,	8	},
{	23	,	1.6	,	9	},
{	24	,	1.6	,	10	},
{	25	,	1.6	,	11	},
{	26	,	1.6	,	12	},
{	27	,	1.8	,	9	},
{	28	,	1.8	,	10	},
{	29	,	1.8	,	11	},
{	30	,	1.8	,	12	},
{	31	,	2	,	9	},
{	32	,	2	,	10	},
{	33	,	2	,	11	},
{	34	,	2	,	12	},
{	35	,	2.2	,	10	},
{	36	,	2.2	,	12	},
{	37	,	2.4	,	10	},
{	38	,	2.4	,	11	},
{	39	,	2.4	,	12	},
{	40	,	2.6	,	10	},
{	41	,	2.6	,	11	},
{	42	,	2.6	,	12	},
{	43	,	2.8	,	10	},
{	44	,	2.8	,	11	},
{	45	,	2.8	,	12	},
{	46	,	3	,	11	},
{	47	,	3	,	12	},
{	48	,	3.2	,	11	},
{	49	,	3.2	,	12	},
{	50	,	3.201	,	10	},
{	51	,	3.201	,	11	},
{	52	,	3.201	,	12	}
#else
{	0	,	1.2	,	4	},
{	1	,	1.4	,	4	},
{	2	,	1.6	,	4	},
{	3	,	1.8	,	4	},
{	4	,	2.0	,	4	},
{	5	,	2.2	,	4	},
{	6	,	2.4	,	4	},
{	7	,	2.6	,	4	},
{	8	,	2.8	,	4	},
{	9	,	3.0	,	4	},
{	10	,	3.2	,	4	},
{	11	,	3.201	,	4	}
#endif
};

double x_hat_minus = 0.0;
double x_hat = .2;
double Q = .00001;
double P = 1.0;
double P_minus = 0.0;
double H = 0.0;
double R = .01;
double K = 0.0; 

typedef struct {  
  double x_hat_minus;
  double x_hat;
  double Q;
  double P;
  double P_minus;
  double H;
  double R;
  double K;
} kalman_filter_t;

kalman_filter_t filter_init = {0.0, 0.3125, 0.00001, 1.0, 0.0, 0.0, 0.01, 0.0};

#define HEURISTIC_W 0

int adaptation_level = 1;

kalman_filter_t power_filter = {0.0, 0.2, 0.00001, 1.0, 0.0, 0.0, 0.01, 0.0};
kalman_filter_t speed_filter = {0.0, 0.3125, 0.00001, 1.0, 0.0, 0.0, 0.01, 0.0};

double get_base_value(kalman_filter_t* k,
		      double current_measurement, 
		      double last_coeff) {
  
  k->x_hat_minus = k->x_hat;
  k->P_minus = k->P + k->Q;
  
  k->H = last_coeff;
  k->K = k->P_minus * k->H /( k->H * k->P_minus* k->H + k->R);
  k->x_hat = k->x_hat_minus + k->K * (current_measurement - k->H * k->x_hat_minus);
  k->P = (1 - k->K * k->H) * k->P_minus;
  
  return 1.0/k->x_hat;
}

const double MAX_COST = 7.40;

void get_power_signal(double current_power, 
		      double desired_power,
		      double base_power,
		      double* e,
		      double* eo,
		      double* u,
		      double* uo,
		      double* uoo) {

  if(current_power < desired_power) {
    p1 = 0.1;
    p2 = 0.8;
    z1 = 0.2;
    p1 = 0;
    p2 = 0;
    z1 = 0;
  }
  else {
    p1 = 0;
    p2 = 0;
    z1 = 0;

  }

  
  double A         = -(-p1*z1 - p2*z1 + mu*p1*p2 - mu*p2 + p2 - mu*p1 + p1 + mu);
  double B         = -(-mu*p1*p2*z1 + p1*p2*z1 + mu*p2*z1 + mu*p1*z1 - mu*z1 - p1*p2);
  double C         = ((mu - mu*p1)*p2 + mu*p1 - mu)*base_power;
  double D         = ((mu*p1-mu)*p2 - mu*p1 + mu)*base_power*z1;
  double F         = 1/(z1-1);
  
  double y = current_power;
  *e = desired_power - y;
  
  //printf("u = %f, uo = %f, e = %f, A = %f, B= %f, C = %f, D= %f, F = %f\n",
  // *u, *uo, *e, A, B, C, D, F);
  
  *u = F * (A*(*uo) + B*(*uoo) + C*(*e) + D*(*eo));

  
  
  if (*u<1)  *u=1;        
  if (*u>MAX_COST) *u=MAX_COST;  
  *uoo = *uo;            // saving values
  *uo  = *u;
  *eo  = *e;
}

double max_speedup;

void get_speed_signal(double current_speed, 
		      double desired_speed,
		      double base_speed,
		      double* e,
		      double* eo,
		      double* u,
		      double* uo,
		      double* uoo) {
  //double p1 = 0.1;
  //double p2 = 0.2;
  //double z1 = 0.15; 

  double A         = -(-p1*z1 - p2*z1 + mu*p1*p2 - mu*p2 + p2 - mu*p1 + p1 + mu);
  double B         = -(-mu*p1*p2*z1 + p1*p2*z1 + mu*p2*z1 + mu*p1*z1 - mu*z1 - p1*p2);
  double C         = ((mu - mu*p1)*p2 + mu*p1 - mu)*base_speed;
  double D         = ((mu*p1-mu)*p2 - mu*p1 + mu)*base_speed*z1;
  double F         = 1/(z1-1);
  
  double y = current_speed;
  *e = desired_speed - y;
  
  
  //*u = F * (A*(*uo) + B*(*uoo) + C*(*e) + D*(*eo));

  *u = *uo + *e/base_speed;

  //  printf("u = %f, uo = %f, e = %f, base_speed = %f\n",
  // *u, *uo, *e, base_speed);


  //double max_speedup = speed_controller_states[NAPPSTATES-1].speedup * 
  //power_controller_states[NSYSSTATES-1].speedup;
  if (*u<1)  *u=1;        
  if (*u>max_speedup) *u=max_speedup;  
  *uoo = *uo;            // saving values
  *uo  = *u;
  *eo  = *e;
}


int power_translate(double u, 
		    const control_state_t* states, 
		    int nstates) {
  int i;

  control_state_t cs = states[nstates-1];

  double efficiency = cs.speedup/cs.cost;
  int index = nstates-1;


  for(i = 0; i < nstates; i++) {
    if(states[i].cost >= u) {

      double new_eff = states[i].speedup/states[i].cost;
      if(new_eff > efficiency) {

	cs = states[i];
	efficiency = new_eff;
	index = i;
      }	
    }
  }

  return index;
}

int power_translate2(double u, 
		     const control_state_t* states, 
		     int state1,
		     const power_state_t* pstates,
		     int nstates) {
  int i;
  
  control_state_t cs = states[0];
  
  double efficiency = cs.speedup;///cs.cost;
    int index = 0;
    
    for(i = 0; i < nstates; i++) {
      if(states[i].cost < u && pstates[state1].cores == pstates[i].cores /* pstates[state1].freq == pstates[i].freq*/) {
	
	double new_eff = states[i].speedup;///states[i].cost;
	  if(new_eff >= efficiency) {
	    
	    cs = states[i];
	    efficiency = new_eff;
	    index = i;
	  }	
      }
    }
    
    return index;
}

int speed_translate(double u, 
		    const control_state_t* states, 
		    int nstates) {
  int i;

  control_state_t cs = states[nstates-1];

  double efficiency = cs.speedup/(cs.cost+1);
  int index = nstates-1;


  for(i = 0; i < nstates; i++) {
    if(states[i].speedup >= u) {

      //index = i;
      //break;
      double new_eff = states[i].speedup/(states[i].cost+1);
      if(new_eff > efficiency) {

      	cs = states[i];
	efficiency = new_eff;
	index = i;
      }	
    }
  }

  return index;
}

int speed_translate2(double u, 
		     const control_state_t* states, 
		     int nstates) {
  int i;
  
  control_state_t cs = states[0];
  
  double efficiency = cs.speedup/(cs.cost+1);
    int index = 0;
    
    for(i = 0; i < nstates; i++) {
      if(states[i].speedup < u) {
	
	double new_eff = states[i].speedup/(states[i].cost+1);
	  if(new_eff >= efficiency) {
	    
	    cs = states[i];
	    efficiency = new_eff;
	    index = i;
	  }	
      }
    }
    
    return index;
}


int last_speed_state = 0;	
int control_nref = 5;
int control_range = 16;
int control_refine = 7;

void speed_apply(int state,
		 speed_state_t* ps,
		 int nstates) {
  
  char command[4096];
  int i;
  
  //printf("APPLYING SPPED STATE %d\n", state);

  if(last_speed_state ==state)
    return;
  else 
    last_speed_state = state;

#if 0
  control_nref = ps[state].ref;
  control_range = ps[state].range;
  control_refine = ps[state].subme;
#endif
  //sprintf(command,"echo \"%d\n%d\n%d\n\" >  x264_settings",
  //	  ps[state].ref,ps[state].range,ps[state].subme);
  //system(command);
  
}

volatile int last_power_state[1] = {52};
volatile int current_power_state[1] = {52};
int last_cores = 12;

int power_apply(int state_id,
		pid_t* pid,
		int npids,
		const power_state_t* states,
		int nstates) {
  
  int i;
  
  int rc = 0;
  int retvalsyscall1 = 0;
  int retvalsyscall2 = 0;
  power_state_t state = states[0];
  char command[4096];
  
#if 0
  if(state_id == last_power_state[0])
    return 0;
  else
    last_power_state[0] = state_id;
#endif  

  for(i = 0; i < nstates; i++) {
    if( states[i].id == state_id) {
      state = states[i];
      break;
    }
  }


#if 1
  
  sprintf(command,"taskset -p -c 0-%d  %d > /dev/null",state.cores-1, getpid());
  retvalsyscall1 = system(command);

#endif 
  
  freqsetting[0] = state.freq * 1000000;

  printf("Freq setting = %d\n", freqsetting[0]);

  current_power_state[0] = state_id;

#if 1
  for(i = 0; i < 12; i++) {
    sprintf(command,
	    "echo %d > /sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", 
	    (int) (state.freq * 1000000), i);
    
    retvalsyscall2 = system(command);
    if(retvalsyscall2 != 0)
      fprintf(stderr, "ERROR setting clocks: %d\n", retvalsyscall2);
  }
#endif
  
  if(retvalsyscall1 || retvalsyscall2) {
    rc = -1;
  }

  return rc;
}
//extern int window_size;
double hr[NSYSSTATES];
double pwr[NSYSSTATES];
__thread char freqcommand[4096];

//----------------------------------------------------
void* do_work(void* args) {
  volatile thread_arg_t* arg = (thread_arg_t*) args;

  volatile int* count      = arg->d_count;
  volatile int* global_min = arg->global_min;
  volatile int* local_min  = arg->local_min;
  int tid     = arg->tid;
  int  P      = arg->P;
  int* Q      = arg->Q;
  int* D      = arg->D;
  const int N = arg->N;
  int i;
  int local_count = N;
  int thread_counter = 0;
  int curfreq = 3201000;

  int i_start =  tid    * arg->N / (arg->P);
  int i_stop  = (tid+1) * arg->N / (arg->P);

  while(local_count != 0) {
    local_min[tid] = get_local_min(Q, D, i_start, i_stop);
    //printf("Thread %d: Waiting at barrier\n", tid);
    //user_barrier(arg->ubarrier, tid);
    pthread_barrier_wait(arg->barrier1);
    
    if(tid == 0) {

      int min = INT_MAX;
      for(i = 0; i < P; i++) {
	if(local_min[i] < min)
	  min = local_min[i];
      }
      *count = *count - 1;
      *global_min = min;
      Q[*global_min] = 0;
    }
    //printf("Thread %d: Waiting at barrier\n", tid);
    pthread_barrier_wait(arg->barrier1);
    //user_barrier(arg->ubarrier, tid);
    //#pragma omp barrier
    //sched_setaffinity(0, sizeof(cpu_set_t), cpumask);
    //if(thread_counter % 200 == 0) {
 
#if 0     
    if(*freqsetting != curfreq) {

      sprintf(freqcommand,
	      "echo %d > /sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", 
	      (int) ( freqsetting[0]), tid);

      //printf("%s\n", freqcommand);
      int retvalsyscall2 = system(freqcommand);
      printf("local count = %d, freq = %d, cur = %d\n", local_count, *freqsetting, curfreq);
      curfreq = freqsetting[0];
    }
#endif
    //thread_counter++;
    int u = *global_min;
    //printf("Thread %d: Count is %d, global_min is %d\n", tid, *count, u);
    
    for(i = i_start; i < i_stop; i++) {
      relax(u, i, D, W);
    }
    //for(i = 0; i < N; i++) {
    //  printf("%12d ", D[i]);
    //}
    //printf("\n");
    //user_barrier(arg->ubarrier, tid);
    pthread_barrier_wait(arg->barrier1);
    local_count--;
    //printf("Thread %d: Count is %d\n", tid, *count);
  }
  
  return NULL;

}

#define ITERS 20000

void start_power_mon(void) {
  char command[4096];
  sprintf(command,
	  "/afs/csail.mit.edu/u/h/hank/perforation.cag/tools/powerQoS/pyWattsup-hank.py start >& /dev/null");

  system(command);
}

int main(int argc, char** argv) {
  int i,j,k;
  const int N = atoi(argv[2]);
  double wtime;
  double wtime1;
  double wtime2;
  int* D;
  int* Q;
  int d_count = N;
  const int P = atoi(argv[1]);
  pthread_barrier_t barrier1;
  pthread_barrier_t barrier2;
  user_barrier_t ubarrier;
  float min_heartrate;
  float max_heartrate;
  hr[0] = 16.4;
  pwr[0] = 16;
  
  int n;
  for(n =1; n < NSYSSTATES; n++) {
    hr[n]  = power_controller_states[n].speedup*hr[0];
    pwr[n] = power_controller_states[n].cost*pwr[0];
  }
 
  if(getenv("DIJKSTRA_MIN_HEART_RATE") == NULL) {
    min_heartrate = 0.0;
  }
  else
    min_heartrate = atof(getenv("DIJKSTRA_MIN_HEART_RATE"));
  
  if(getenv("DIJKSTRA_MAX_HEART_RATE") == NULL) {
    max_heartrate = 100.0;
  }
  else          max_heartrate = atof(getenv("DIJKSTRA_MAX_HEART_RATE"));
  
  if(getenv("DIJKSTRA_WINDOW_SIZE") == NULL) {
    window_size = 20;
  }
  else
    window_size = atoi(getenv("DIJKSTRA_WINDOW_SIZE"));

  if(getenv("DIJKSTRA_POWER_TARGET") == NULL) {
    power_target = 70;
  }
  else
    power_target = atof(getenv("DIJKSTRA_POWER_TARGET"));



 
  printf("init heartbeat with %f %f %d\n", power_target, max_heartrate, window_size);
  
  heartbeat_init(&heart,
		 min_heartrate, max_heartrate,
		 0, 1,
		 0, 150,
		 window_size, 1, "dijkstra_heartbeat.log");
  //window_size, 100, NULL);
 
  printf("heartbeat init'd\n");
 
  D = (int*) malloc(N*sizeof(int));
  Q = (int*) malloc(N*sizeof(int));


  printf("ALLOCD D&Q\n");

  W = (INTTYPE**) malloc(N*sizeof(INTTYPE*));
  for(i = 0; i < N; i++) {
    W[i] = (INTTYPE*) malloc(N*sizeof(INTTYPE));
    for(j = 0; j < N; j++) {
      if( N != 5) {
	double v = drand48();
	
	if(v < 0.8)
	  W[i][j] = (INTTYPE) (v*100) + 1;
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

  printf("ALLOCD W\n");
  //  start_power_mon();

  pthread_barrier_init(&barrier1,NULL,P);
  pthread_barrier_init(&barrier2,NULL,P);

  user_barrier_init(&ubarrier, P);


  for(k = 0; k < ITERS; k++) {

  initialize_single_source(W,D,Q,0,N);

  if(hb_counter % 1 == 0) {
    heartbeat(&heart,0,1);
    
    //if(hb_counter > 20000) exit(0);
    
    
#if 0 // Power and performance control
    int PERIOD = window_size;
    heartbeat_record_t hbr;
    hb_get_current(&heart, &hbr);
    
    double power_goal = power_target;
    double speed_goal = 1.4;
    
    //if(hbr.beat >= 100) {/*power_goal = 35.0;*/ speed_goal = 4.615;}
    //if(hbr.beat >= 500) {speed_goal = 14.8;}
    //if(hbr.beat >= 1000) {speed_goal = 10.6;}
    
    //	printf("power goal is 70\n");
#if 0
#define ALPHA 0.85
    int ready = 1;				  
    if(ready) {
      hr[last_power_state[0]] =ALPHA*hbr.instant_rate + (1.0-ALPHA)*hr[last_power_state[0]];
      
      double xyz = hbr.instant_power;
      //printf("POWER = %f %f\n", xyz, );
      pwr[last_power_state[0]] = ALPHA*xyz + (1.0-ALPHA)*pwr[last_power_state[0]];
      
      double xy = (hr[last_power_state[0]]/hr[0]);
      power_controller_states[last_power_state[0]].speedup = (xy);
      xy = pwr[last_power_state[0]]/pwr[0];
      
      power_controller_states[last_power_state[0]].cost =xyz;
      //last_speedup = (controller_states[state_1].cost * tmax + 90.0/143.0*tidle)/tau;
    }
#endif
    
    int old_action = __sync_fetch_and_add(&current_action,1);
    if(old_action == PERIOD - 1) {
      current_action = 0;
    }
    if( old_action == 0) {
      double act_power = hbr.window_power;
      double act_speed = hbr.window_rate;
      
      double base_power = get_base_value(&power_filter, act_power, last_powerup);
      
      get_power_signal(act_power,
		       power_goal,
		       base_power,
		       &power_e,
		       &power_eo,
		       &power_u,
		       &power_uo,
		       &power_uoo);
      
      double act_power_up = last_powerup = power_u;
      
      power_tdo_state_1 = power_translate(power_u, 
					  power_controller_states, 
					  (int) NSYSSTATES);
      power_tdo_state_2 = power_translate2(power_u, 
					   power_controller_states,
					   power_tdo_state_1,
					   power_states,
					   (int) NSYSSTATES);
      
      //power_tdo_state_1 = NSYSSTATES-1; 
      //power_tdo_state_2 = 0;//16;//NSYSSTATES-1;
      
      if(power_controller_states[power_tdo_state_1].speedup 
	 <= power_controller_states[power_tdo_state_2].speedup) {
	power_tdo_state_1 = power_tdo_state_2;
      }
      
      double power_time1;
      
      if(power_controller_states[power_tdo_state_1].cost == 
	 power_controller_states[power_tdo_state_2].cost ||
	 power_u == power_controller_states[power_tdo_state_1].cost) 
	power_time1 = 1.0;
      else
	power_time1 = (power_u - 
		       power_controller_states[power_tdo_state_2].cost) /
	  (power_controller_states[power_tdo_state_1].cost - 
	   power_controller_states[power_tdo_state_2].cost);
      
      double power_time2 = 1.0 - power_time1;
      
      h1 = power_time1*PERIOD+0.5;
      h2 = PERIOD-h1;
      
#if 1
	  printf("\nCONTROL beat=%lld, power=%f, des=%f, w=%f, u=%f, uo=%f, state=%d, state2=%d, %f, %f, %f, %f, %d, %d\n\n",
		 hbr.beat,act_power,power_goal,1.0/base_power,power_u,last_powerup,power_controller_states[power_tdo_state_1].id, power_controller_states[power_tdo_state_2].id,  power_controller_states[power_tdo_state_1].cost, power_controller_states[power_tdo_state_2].cost, power_time1, power_time2, h1, h2); 
	  
#endif
    printf("%d %f %f %f %f    %f %f    %d %f %f %f %f\n", 
	   hbr.beat, 
	   act_power, 
	   hbr.window_rate*70.0,
	   1.0/base_power,
	   power_u,
	   power_time1,
	   power_time2,
	   power_tdo_state_1,
	   ((float)((float) power_states[power_tdo_state_1].cores * power_time1
		    + (float) power_states[power_tdo_state_2].cores * power_time2))/12.0,
	   (power_states[power_tdo_state_1].freq*power_time1
	    + power_states[power_tdo_state_2].freq*power_time2)/3.201, 
	   0.0,
	   ((power_tdo_state_1 >= 50 ? 1.0 : 0.0)*power_time1 + 
	    (power_tdo_state_2 >= 50 ? 1.0 : 0.0) *power_time2));

	  
	  //if(npids == 0) {
	  //npids = get_all_subpids(getpid(), pids);
	  //}
	  power_apply(power_tdo_state_1,
		      pids,
		      npids,
		      power_states,
		      NSYSSTATES);
	  
	  if(h1 == 0)
	    power_apply(power_tdo_state_2,
			pids,
			npids,
			power_states,
			NSYSSTATES);
				  }
	
	if (old_action == h1) {
	  //if(npids == 0) {
	  //  npids = get_all_subpids(getpid(), pids);
	  //}
	  power_apply(power_tdo_state_2,
		      pids,
		      npids,
		      power_states,
		      NSYSSTATES);
	  
	}
	
#endif
	
  }
  hb_counter++;

      if(N <= 20) {
	for(i = 0; i < N; i++) {
	  printf("%12d ", D[i]);
	}
	printf("\n");
      }
      
      //printf("P  = %d\n", P);

  // wtime1 = omp_get_wtime ( );
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
  // wtime2 = omp_get_wtime ( );
  //wtime = wtime2 - wtime1;
  }
  if(N <= 20) {
    for(i = 0; i < N; i++) {
      printf("%12d ", D[i]);
    }
    printf("\n");
  }
  //printf ( "Time     = %e\n", wtime );
  heartbeat_finish(&heart);
}

int initialize_single_source(INTTYPE** W, 
			     int*  d, 
			     int*  q, 
			     int   source,
			     int   N) {
  int i;

  for(i = 0; i < N; i++) {
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

  for(i = start; i < stop; i++) {
    if(D[i] < min && Q[i]) {
      min = i;
    }
  }
  return min;

}

#include <math.h>

int relax(int u, int i, volatile int* D, INTTYPE** W) {
  int j;
  if(D[i] > D[u] + W[u][i]) {
    //for(j = 0; j < 1000; j++) 
    D[i] = D[u] + W[u][i];
  }

  return 0;
}
