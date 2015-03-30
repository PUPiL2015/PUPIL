


/*-----------------------------------------------------------------------*/
/* Program: Stream                                                       */
/* Revision: $Id: stream.c,v 5.9 2009/04/11 16:35:00 mccalpin Exp mccalpin $ */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*                                                                       */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
/* Copyright 1991-2005: John D. McCalpin                                 */
/*-----------------------------------------------------------------------*/
/* License:                                                              */
/*  1. You are free to use this program and/or to redistribute           */
/*     this program.                                                     */
/*  2. You are free to modify this program for your own use,             */
/*     including commercial use, subject to the publication              */
/*     restrictions in item 3.                                           */
/*  3. You are free to publish results obtained from running this        */
/*     program, or from works that you derive from this program,         */
/*     with the following limitations:                                   */
/*     3a. In order to be referred to as "STREAM benchmark results",     */
/*         published results must be in conformance to the STREAM        */
/*         Run Rules, (briefly reviewed below) published at              */
/*         http://www.cs.virginia.edu/stream/ref.html                    */
/*         and incorporated herein by reference.                         */
/*         As the copyright holder, John McCalpin retains the            */
/*         right to determine conformity with the Run Rules.             */
/*     3b. Results based on modified source code or on runs not in       */
/*         accordance with the STREAM Run Rules must be clearly          */
/*         labelled whenever they are published.  Examples of            */
/*         proper labelling include:                                     */
/*         "tuned STREAM benchmark results"                              */
/*         "based on a variant of the STREAM benchmark code"             */
/*         Other comparable, clear and reasonable labelling is           */
/*         acceptable.                                                   */
/*     3c. Submission of results to the STREAM benchmark web site        */
/*         is encouraged, but not required.                              */
/*  4. Use of this program or creation of derived works based on this    */
/*     program constitutes acceptance of these licensing restrictions.   */
/*  5. Absolutely no warranty is expressed or implied.                   */
/*-----------------------------------------------------------------------*/
# include <stdio.h>
# include <math.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>
#include <stdlib.h>
 //#include <numa.h> 
 //#include <numaif.h> 

#include <errno.h>

#include <heartbeat-accuracy-power.h>
#include <string.h>

#include <sched.h>

 //#include "states.h"


/* INSTRUCTIONS:
 *
 *	1) Stream requires a good bit of memory to run.  Adjust the
 *          value of 'N' (below) to give a 'timing calibration' of 
 *          at least 20 clock-ticks.  This will provide rate estimates
 *          that should be good to about 5% precision.
 */

#ifndef N
#   define N	20000000
#endif
#ifndef NTIMES
#   define NTIMES	500
#endif
#ifndef OFFSET
#   define OFFSET	13
#endif

/*
 *	3) Compile the code with full optimization.  Many compilers
 *	   generate unreasonably bad code before the optimizer tightens
 *	   things up.  If the results are unreasonably good, on the
 *	   other hand, the optimizer might be too smart for me!
 *
 *         Try compiling with:
 *               cc -O stream_omp.c -o stream_omp
 *
 *         This is known to work on Cray, SGI, IBM, and Sun machines.
 *
 *
 *	4) Mail the results to mccalpin@cs.virginia.edu
 *	   Be sure to include:
 *		a) computer hardware model number and software revision
 *		b) the compiler flags
 *		c) all of the output from the test case.
 * Thanks!
 *
 */

# define HLINE "-------------------------------------------------------------\n"

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif


 //static double	a[N+OFFSET],
 //		b[N+OFFSET],
 //		c[N+OFFSET];

 //double a;
 //static double* b;
 //static double* c;

static double	avgtime[4] = {0}, maxtime[4] = {0},
		mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

double* a;
double* b;
double* c;
double* sa;
double* sb;
double* sc;
double* oneMca;
double* oneMcb;
double* oneMcc;
double* twoMca;
double* twoMcb;
double* twoMcc;

static char	*label[4] = {"Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

static double	bytes[4] = {
    2 * sizeof(double) * N,
    2 * sizeof(double) * N,
    3 * sizeof(double) * N,
    3 * sizeof(double) * N
    };

extern double mysecond();
extern void checkSTREAMresults();
#ifdef TUNED
extern void tuned_STREAM_Copy();
extern void tuned_STREAM_Scale(double scalar);
extern void tuned_STREAM_Add();
extern void tuned_STREAM_Triad(double scalar);
#endif
#ifdef _OPENMP
extern int omp_get_num_threads();
#endif


/*-----------------------------------------------------------------------*/
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
#define STARTING_POWERUP 1.9
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
//#define NSYSSTATES 12
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
#if 0
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
#if 0
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
{	0	,	1.2	,	1	},
{	1	,	1.4	,	1	},
{	2	,	1.6	,	1	},
{	3	,	1.8	,	1	},
{	4	,	2.0	,	1	},
{	5	,	2.2	,	1	},
{	6	,	2.4	,	1	},
{	7	,	2.6	,	1	},
{	8	,	2.8	,	1	},
{	9	,	3.0	,	1	},
{	10	,	3.2	,	1	},
{	11	,	3.201	,	1	}
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

const double MAX_COST = 7.5;

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
      if(states[i].cost < u /*&& pstates[state1].cores == pstates[i].cores*/ /* pstates[state1].freq == pstates[i].freq*/) {
	
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

volatile int last_power_state[1] = {16};
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
  
  if(state_id == last_power_state[0])
    return 0;
  else
    last_power_state[0] = state_id;
  
  for(i = 0; i < nstates; i++) {
    if( states[i].id == state_id) {
      state = states[i];
      break;
    }
  }


#if 0
  
  sprintf(command,"taskset -p -c 0-%d  %d > /dev/null",state.cores-1, getpid());
  retvalsyscall1 = system(command);
  for( i = 1; i < 12; i++) {
    sprintf(command,"taskset -p -c 0-%d  %d > /dev/null",state.cores-1, pid[i]);
    //printf("%s\n", command);
    retvalsyscall1 = system(command);
  }

#endif 
  
  freqsetting[0] = state.freq * 1000000;

  printf("Freq setting = %d\n", freqsetting[0]);

  //current_power_state[0] = state_id;

#if 0
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
heartbeat_t heart;
int window_size;
double hr[NSYSSTATES];
double pwr[NSYSSTATES];
//__thread char freqcommand[4096];

//----------------------------------------------------


int
main()
    {
    int			quantum, checktick();
    int			BytesPerWord;
    register int	j, k;
    double		scalar, t, times[4][NTIMES];

    /* --- SETUP --- determine precision and check timing --- */

    //    unsigned long mask = 0x01;
    //    unsigned long q = 0xFFFFFFFFFFFFFFFF;
    unsigned long q = 0x0;
    int mode = 0;

    unsigned long slow_mask = 0x01;
    unsigned long fast_mask = 0x00;

    void** addrs;
    int* nodes; 
    int* status; 
    long long int ii;

    a = malloc(N*sizeof(double));
    b = malloc(N*sizeof(double));
    c = malloc(N*sizeof(double));
    
    float min_heartrate;
    float max_heartrate; 
    //int window_size;
    //UMAX = get_max_speedup();
   
    hr[0] = 4.17;
    pwr[0] = 21.05;
    
    int n;
    for(n =1; n < NSYSSTATES; n++) {
      hr[n]  = power_controller_states[n].speedup*hr[0];
      pwr[n] = power_controller_states[n].cost*pwr[0];
    }
     cpu_set_t cpuset;

    //CPU_ZERO(&cpuset);
    //CPU_SET(0, &cpuset);
    //sched_setaffinity(0, 8, &cpuset);

#define GOAL 9.212559

    if(getenv("STREAM_MIN_HEART_RATE") == NULL) {
      min_heartrate = GOAL; //2.345;//4.54;
    }
    else
      min_heartrate = atof(getenv("STREAM_MIN_HEART_RATE"));
    
    if(getenv("STREAM_MAX_HEART_RATE") == NULL) {
      max_heartrate = GOAL;//4.54;
    }
    else
      max_heartrate = atof(getenv("STREAM_MAX_HEART_RATE"));
    
    if(getenv("STREAM_WINDOW_SIZE") == NULL) {
      window_size = 50;
    }
    else
      window_size = atoi(getenv("STREAM_WINDOW_SIZE"));
  
    int PERIOD = window_size;

    max_actions = window_size;
    printf("init heartbeat with %f %f %d\n", min_heartrate, max_heartrate, window_size);
    
    heartbeat_init(&heart, 
		   min_heartrate, max_heartrate, 
		   0,1,
		   0,150,
		   window_size, 1, "STREAM_heartbeat.log");
    
    printf("heartbeat init'd\n");
    
    printf(HLINE);
    printf("STREAM version $Revision: 5.9 $\n");
    printf(HLINE);
    BytesPerWord = sizeof(double);
    printf("This system uses %d bytes per DOUBLE PRECISION word.\n",
	BytesPerWord);

    printf(HLINE);
#ifdef NO_LONG_LONG
    printf("Array size = %d, Offset = %d\n" , N, OFFSET);
#else
    printf("Array size = %llu, Offset = %d\n", (unsigned long long) N, OFFSET);
#endif

    printf("Total memory required = %.1f MB.\n",
	(3.0 * BytesPerWord) * ( (double) N / 1048576.0));
    printf("Each test is run %d times, but only\n", NTIMES);
    printf("the *best* time for each is used.\n");

#ifdef _OPENMP
    printf(HLINE);
#pragma omp parallel 
    {
#pragma omp master
	{
	    k = omp_get_num_threads();
	    printf ("Number of Threads requested = %i\n",k);
        }
    }
#endif

    printf(HLINE);
#pragma omp parallel
    {
    printf ("Printing one line per active thread....\n");
    }

    /* Get initial value for system clock. */
#pragma omp parallel for
    for (j=0; j<N; j++) {
	a[j] = 1.0;
	b[j] = 2.0;
	c[j] = 0.0;
	}

    printf(HLINE);

    if  ( (quantum = checktick()) >= 1) 
	printf("Your clock granularity/precision appears to be "
	    "%d microseconds.\n", quantum);
    else {
	printf("Your clock granularity appears to be "
	    "less than one microsecond.\n");
	quantum = 1;
    }

    t = mysecond();
#pragma omp parallel for
    for (j = 0; j < N; j++)
	a[j] = 2.0E0 * a[j];
    t = 1.0E6 * (mysecond() - t);

    printf("Each test below will take on the order"
	" of %d microseconds.\n", (int) t  );
    printf("   (= %d clock ticks)\n", (int) (t/quantum) );
    printf("Increase the size of the arrays if this shows that\n");
    printf("you are not getting at least 20 clock ticks per test.\n");

    printf(HLINE);

    printf("WARNING -- The above is only a rough guideline.\n");
    printf("For best results, please be sure you know the\n");
    printf("precision of your system timer.\n");
    printf(HLINE);
    
    /*	--- MAIN LOOP --- repeat test cases NTIMES times --- */

    for (k=0; k<NTIMES; k++)
    {
      /*-----------------------------------------------------*/
      // Control
      //printf("HEartbeat\n");
      heartbeat(&heart, k, 1);
      heartbeat_record_t hbr;
      hb_get_current(&heart, &hbr);
  #if 0// Power and performance control
	int PERIOD = window_size;
	heartbeat_record_t hbr;
	hb_get_current(&heart, &hbr);
	
	double power_goal = 30;
	double speed_goal = 1.4;

#if 0
#define ALPHA 0.85
    int ready = 1;				  
    if(ready) {
      hr[*last_power_state] =ALPHA*hbr.instant_rate + (1.0-ALPHA)*hr[*last_power_state];
      
      double xyz = hbr.instant_power;
      //printf("%d: POWER = %f %f\n", *last_power_state, xyz, power_controller_states[*last_power_state].cost);
      pwr[*last_power_state] = ALPHA*xyz + (1.0-ALPHA)*pwr[*last_power_state];
      
      double xy = (hr[*last_power_state]/hr[0]);
      power_controller_states[*last_power_state].speedup = (xy);
      xy = pwr[*last_power_state]/pwr[0];
      
      power_controller_states[*last_power_state].cost =xy;
      //last_speedup = (controller_states[state_1].cost * tmax + 90.0/143.0*tidle)/tau;
    }
#endif	
	if( current_action == 0 && hbr.window_rate != 0.0) {
	  double act_power = hbr.window_power;
	  double act_speed = hbr.window_rate;
	  
	  double base_power;
	  if(act_power > 0) {
	    base_power = get_base_value(&power_filter, act_power, last_powerup);
	  }
	  else base_power = 1.0/power_filter.x_hat;
	  
	  double base_speed = get_base_value(&speed_filter, act_speed, last_speedup);
	  
	  //double app_state = acc_translate();
	  
	  get_power_signal(act_power,
			   power_goal,
			   base_power,
			   &power_e,
			   &power_eo,
			   &power_u,
			   &power_uo,
			   &power_uoo);
	  
	  get_speed_signal(act_speed,
			   speed_goal,
			   1.0/base_speed,
			   &speed_e,
			   &speed_eo,
			   &speed_u,
			   &speed_uo,
			   &speed_uoo);
	  
	  
	  double act_power_up = last_powerup = power_u;
	  
	  double act_speed_up = last_speedup = speed_u;//*power_u;
	  
	  power_tdo_state_1 = power_translate(power_u, 
					      power_controller_states, 
					      (int) NSYSSTATES);
	  power_tdo_state_2 = power_translate2(power_u, 
					       power_controller_states,
					       power_tdo_state_1,
					       power_states,
					       (int) NSYSSTATES);

	  //power_tdo_state_1 = 11;
	  //power_tdo_state_2 = 11;
	  
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
	  //h1 = PERIOD;
	  
	  double sys_speedup = (power_controller_states[power_tdo_state_1].speedup 
				* power_time1 
				+
				power_controller_states[power_tdo_state_2].speedup
				* power_time2);
	  
	  double sub_speedup = speed_u;//sys_speedup;
	  
	  

	  speed_tdo_state_1 = speed_translate(sub_speedup,
					      speed_controller_states,
					      (int) NAPPSTATES);
	  //speed_tdo_state_1 = 17;
	  speed_tdo_state_2 = speed_translate2(sub_speedup,
					       speed_controller_states,
					       (int) NAPPSTATES);
	  //	   speed_tdo_state_2 = 20;
	  double speed_time1;
	  
	  

	  if (hbr.beat >=1000) speed_tdo_state_1 = speed_tdo_state_2 = 51; 
	  //else  speed_tdo_state_1 = speed_tdo_state_2 = 0; 
	  
	  if(speed_controller_states[speed_tdo_state_1].speedup == 
	     speed_controller_states[speed_tdo_state_2].speedup ||
	     sub_speedup == speed_controller_states[speed_tdo_state_1].speedup) 
	    speed_time1 = 1.0;
	  else
	    speed_time1 = (sub_speedup - 
			   speed_controller_states[speed_tdo_state_2].speedup) /
	      (speed_controller_states[speed_tdo_state_1].speedup - 
	       speed_controller_states[speed_tdo_state_2].speedup);
	  
	  
	  double window_time = ((double) PERIOD)/(sub_speedup*base_speed);
	  
	  speed_time1 = (window_time - ((double) PERIOD)/(base_speed*speed_controller_states[speed_tdo_state_2].speedup))
	    /
	    (1.0/(base_speed*speed_controller_states[speed_tdo_state_1].speedup) 
	     - 1.0/(base_speed*speed_controller_states[speed_tdo_state_2].speedup));
	  
	  if(speed_time1 < 0.0) speed_time1 = 0.0;

	  //if(hbr.beat <  500) {speed_tdo_state_1 = 0; speed_time1 = 1.0;}
	  //if(hbr.beat >= 500) {speed_tdo_state_1 = NAPPSTATES-1; speed_time1 = 1.0;}
	  //speed_tdo_state_1 = 27;
	  //speed_time1 = 1.0;
	  
	  double speed_time2 = ((double) PERIOD) - speed_time1;
	  
	  h3 = ((int)(speed_time1+0.5));
	  h4 = PERIOD - h3;
	  
	  
	  // h3 = speed_time1*PERIOD+0.5;
	  //h4 = PERIOD-h3;
	  
	  
#if 0
	  printf("\nCONTROL beat=%lld, power=%f, des=%f, w=%f, u=%f, uo=%f, state=%d, state2=%d, %f, %f, %d, %d\n\n",
		 hbr.beat,act_power,power_goal,1.0/base_power,power_u,last_powerup,power_controller_states[power_tdo_state_1].id, power_tdo_state_2, power_time1, power_time2, h1, h2); 
	  
#endif
	  
	  if(npids == 0) {
	    npids = get_all_subpids(getpid(), pids);
	  }
	  //power_tdo_state_1 = power_tdo_state_2 = 42;
	  power_apply(power_tdo_state_1,
		      pids,
		      npids,
		      power_states,
		      NSYSSTATES);
	  
	  speed_apply(speed_tdo_state_1,
		      speed_states,
		      NAPPSTATES);
	  
	  if(h1 == 0)
	    power_apply(power_tdo_state_2,
			pids,
			npids,
			power_states,
			NSYSSTATES);
	  if(h3 == 0)
	    speed_apply(speed_tdo_state_2,
	  		speed_states,
	  		NAPPSTATES);
	}
	
	if (current_action == h1) {
	  // if(npids == 0) {
	  //npids = get_all_subpids(getpid(), pids);
	  //}
	  //printf("APPLYING 2\n");
	  power_apply(power_tdo_state_2,
		      pids,
		      npids,
		      power_states,
		      NSYSSTATES);
	  
	}
	if (current_action == h3) {
	  speed_apply(speed_tdo_state_2,
	  	      speed_states,
	  	      NAPPSTATES);
	}
	
	//	else if (current_action == PERIOD) {
	// if(npids == 0) {
	  //npids = get_all_subpids(getpid(), pids);
	//}
	//power_apply(power_tdo_state_1,
	//      pids,
	//      npids,
	//      power_states,
	//      NSYSSTATES);
	
	//}
	//else if (current_action == h1+PERIOD) {
	// if(npids == 0) {
	//npids = get_all_subpids(getpid(), pids);
	//}
	//power_apply(power_tdo_state_2,
	//      pids,
	//      npids,
	//      power_states,
	//      NSYSSTATES);
	
	//}
	
	
	current_action++;
	if(current_action == PERIOD) {
	  current_action = 0;
	}
      
#endif
	  
	times[0][k] = mysecond();
#ifdef TUNED
        tuned_STREAM_Copy();
#else
#pragma omp parallel for
	for (j=0; j<N; j++)
	    c[j] = a[j];
#endif
	times[0][k] = mysecond() - times[0][k];
	
	times[1][k] = mysecond();
#ifdef TUNED
        tuned_STREAM_Scale(scalar);
#else
#pragma omp parallel for
	for (j=0; j<N; j++)
	    b[j] = scalar*c[j];
#endif
	times[1][k] = mysecond() - times[1][k];
	
	times[2][k] = mysecond();
#ifdef TUNED
        tuned_STREAM_Add();
#else
#pragma omp parallel for
	for (j=0; j<N; j++)
	    c[j] = a[j]+b[j];
#endif
	times[2][k] = mysecond() - times[2][k];
	
	times[3][k] = mysecond();
#ifdef TUNED
        tuned_STREAM_Triad(scalar);
#else
#pragma omp parallel for
	for (j=0; j<N; j++)
	    a[j] = b[j]+scalar*c[j];
#endif
	times[3][k] = mysecond() - times[3][k];
	}

    /*	--- SUMMARY --- */

    for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
	{
	for (j=0; j<4; j++)
	    {
	    avgtime[j] = avgtime[j] + times[j][k];
	    mintime[j] = MIN(mintime[j], times[j][k]);
	    maxtime[j] = MAX(maxtime[j], times[j][k]);
	    }
	}
    
    printf("Function      Rate (MB/s)   Avg time     Min time     Max time\n");
    for (j=0; j<4; j++) {
	avgtime[j] = avgtime[j]/(double)(NTIMES-1);

	printf("%s%11.4f  %11.4f  %11.4f  %11.4f\n", label[j],
	       1.0E-06 * bytes[j]/mintime[j],
	       avgtime[j],
	       mintime[j],
	       maxtime[j]);
    }
    printf(HLINE);

    /* --- Check Results --- */
    checkSTREAMresults();
    printf(HLINE);
    heartbeat_finish(&heart);
    int i = 0;
    for(i = 0; i < NSYSSTATES; i++ ) {
      printf("%d:  %f  %f\n", i, power_controller_states[i].speedup, power_controller_states[i].cost);
    }

}

# define	M	20

int
checktick()
    {
    int		i, minDelta, Delta;
    double	t1, t2, timesfound[M];

/*  Collect a sequence of M unique time values from the system. */

    for (i = 0; i < M; i++) {
	t1 = mysecond();
	while( ((t2=mysecond()) - t1) < 1.0E-6 )
	    ;
	timesfound[i] = t1 = t2;
	}

/*
 * Determine the minimum difference between these M values.
 * This result will be our estimate (in microseconds) for the
 * clock granularity.
 */

    minDelta = 1000000;
    for (i = 1; i < M; i++) {
	Delta = (int)( 1.0E6 * (timesfound[i]-timesfound[i-1]));
	minDelta = MIN(minDelta, MAX(Delta,0));
	}

   return(minDelta);
    }



/* A gettimeofday routine to give access to the wall
   clock timer on most UNIX-like systems.  */

#include <sys/time.h>

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

void checkSTREAMresults ()
{
	double aj,bj,cj,scalar;
	double asum,bsum,csum;
	double epsilon;
	int	j,k;

    /* reproduce initialization */
	aj = 1.0;
	bj = 2.0;
	cj = 0.0;
    /* a[] is modified during timing check */
	aj = 2.0E0 * aj;
    /* now execute timing loop */
	scalar = 3.0;
	for (k=0; k<NTIMES; k++)
        {
            cj = aj;
            bj = scalar*cj;
            cj = aj+bj;
            aj = bj+scalar*cj;
        }
	aj = aj * (double) (N);
	bj = bj * (double) (N);
	cj = cj * (double) (N);

	asum = 0.0;
	bsum = 0.0;
	csum = 0.0;
	for (j=0; j<N; j++) {
		asum += a[j];
		bsum += b[j];
		csum += c[j];
	}
#ifdef VERBOSE
	printf ("Results Comparison: \n");
	printf ("        Expected  : %f %f %f \n",aj,bj,cj);
	printf ("        Observed  : %f %f %f \n",asum,bsum,csum);
#endif

#ifndef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#endif
	epsilon = 1.e-8;

	if (abs(aj-asum)/asum > epsilon) {
		printf ("Failed Validation on array a[]\n");
		printf ("        Expected  : %f \n",aj);
		printf ("        Observed  : %f \n",asum);
	}
	else if (abs(bj-bsum)/bsum > epsilon) {
		printf ("Failed Validation on array b[]\n");
		printf ("        Expected  : %f \n",bj);
		printf ("        Observed  : %f \n",bsum);
	}
	else if (abs(cj-csum)/csum > epsilon) {
		printf ("Failed Validation on array c[]\n");
		printf ("        Expected  : %f \n",cj);
		printf ("        Observed  : %f \n",csum);
	}
	else {
		printf ("Solution Validates\n");
	}
}

void tuned_STREAM_Copy()
{
	int j;
#pragma omp parallel for
        for (j=0; j<N; j++)
            c[j] = a[j];
}

void tuned_STREAM_Scale(double scalar)
{
	int j;
#pragma omp parallel for
	for (j=0; j<N; j++)
	    b[j] = scalar*c[j];
}

void tuned_STREAM_Add()
{
	int j;
#pragma omp parallel for
	for (j=0; j<N; j++)
	    c[j] = a[j]+b[j];
}

void tuned_STREAM_Triad(double scalar)
{
	int j;
#pragma omp parallel for
	for (j=0; j<N; j++)
	    a[j] = b[j]+scalar*c[j];
}
