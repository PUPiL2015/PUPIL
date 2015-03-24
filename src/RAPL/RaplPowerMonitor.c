#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>
//#include <asm/msr.h>

#define MSR_RAPL_POWER_UNIT		0x606

/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT	0x610
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PKG_PERF_STATUS		0x613
#define MSR_PKG_POWER_INFO		0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT		0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY			0x63A
#define MSR_PP0_PERF_STATUS		0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT		0x640
#define MSR_PP1_ENERGY_STATUS		0x641
#define MSR_PP1_POLICY			0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT		0x618
#define MSR_DRAM_ENERGY_STATUS		0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO		0x61C

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET	0
#define POWER_UNIT_MASK		0x0F

#define ENERGY_UNIT_OFFSET	0x08
#define ENERGY_UNIT_MASK	0x1F00

#define TIME_UNIT_OFFSET	0x10
#define TIME_UNIT_MASK		0xF000
#define PKG_POWER_LIMIT_LOCK_OFFSET 0x3F
#define PKG_POWER_LIMIT_LOCK_MASK 0x1
#define ENABLE_LIMIT_2_OFFSET 0x2F
#define ENABLE_LIMIT_2_MASK 0x1
#define PKG_CLAMPING_LIMIT_2_OFFSET 0x30
#define PKG_CLAMPING_LIMIT_2_MASK 0x1
#define PKG_POWER_LIMIT_2_OFFSET 0x20
#define PKG_POWER_LIMIT_2_MASK 0x7FFF
#define ENABLE_LIMIT_1_OFFSET 0xF
#define ENABLE_LIMIT_1_MASK 0x1
#define PKG_CLAMPING_LIMIT_1_OFFSET 0x10
#define PKG_CLAMPING_LIMIT_1_MASK 0x1
#define PKG_POWER_LIMIT_1_OFFSET 0x0
#define PKG_POWER_LIMIT_1_MASK 0x7FFF
#define TIME_WINDOW_POWER_LIMIT_1_OFFSET 0x11
#define TIME_WINDOW_POWER_LIMIT_1_MASK 0x7F
#define TIME_WINDOW_POWER_LIMIT_2_OFFSET 0x31
#define TIME_WINDOW_POWER_LIMIT_2_MASK 0x7F


int open_msr(int core) {

  char msr_filename[BUFSIZ];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDWR);
  if ( fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr,"Trying to open %s\n",msr_filename);
      exit(127);
    }
  }

  return fd;
}

long long read_msr(int fd, int which) {

  uint64_t data;

  if ( pread(fd, &data, sizeof data, which) != sizeof data ) {
    perror("rdmsr:pread");
    exit(127);
  }

  return (long long)data;
}

void write_msr(int fd, int which, uint64_t data) {

  if ( pwrite(fd, &data, sizeof data, which) != sizeof data ) {
    perror("wrmsr:pwrite");
    exit(127);
  }
}


int32_t wrmsr(int fd, uint64_t msr_number, uint64_t value)
{
	return pwrite(fd, (const void *)&value, sizeof(uint64_t), msr_number);
}

int32_t rdmsr(int fd, uint64_t msr_number, uint64_t * value)
{
	return pread(fd, (void *)value, sizeof(uint64_t), msr_number);
}


void set_power_limit(int fd, int watts, double pu)
{
/*
	uint32_t setpoint = (uint32_t) ((1 << pu) * watts);
	uint64_t reg = 0;
	rdmsr(fd, MSR_PKG_RAPL_POWER_LIMIT, &reg);
	reg = (reg & 0xFFFFFFFFFFFF0000) | setpoint | 0x8000;
	reg = (reg & 0xFFFFFFFF0000FFFF) | 0xD0000;
	wrmsr(fd, MSR_PKG_RAPL_POWER_LIMIT, reg);
*/

	uint32_t setpoint = (uint32_t) (watts/pu);
	uint64_t reg = 0;
	rdmsr(fd, MSR_PKG_RAPL_POWER_LIMIT, &reg);
	reg = (reg & 0xFFFFFFFFFFFF0000) | setpoint | 0x8000;
	reg = (reg & 0xFFFFFFFF0000FFFF) | 0xD0000;
	wrmsr(fd, MSR_PKG_RAPL_POWER_LIMIT, reg);
            


}


#define CPU_SANDYBRIDGE     42
#define CPU_SANDYBRIDGE_EP  45
#define CPU_IVYBRIDGE       58
#define CPU_IVYBRIDGE_EP    62



void main( int argc, char **argv ) {
  int fd1,fd2;
  long long result1, result2;
  double power_units,energy_units,time_units;
  double package1_before,package1_after,package2_before,package2_after;
  double pp0_before,pp0_after;
  double pp1_before=0.0,pp1_after;
  double dram_before=0.0,dram_after;
  double thermal_spec_power,minimum_power,maximum_power,time_window;
  int cpu_model;
  //printf("Starting\n");
  struct  timeval currentime1,currentime2,beginningtime;
  struct timespec interval_1s,interval_1ms,interval_10ms, interval_100ms, interval_500ms;
  long  double nowtime, power1, power2,tmp_max;
 // char *filename;
  gettimeofday(&beginningtime,NULL);
  //printf("beginningtime.tv_sec= %ld \n",beginningtime.tv_sec);
  //printf("beginningtime.tv_usec= %ld \n",beginningtime.tv_usec);
  interval_500ms.tv_sec = 0;
  interval_500ms.tv_nsec = 500000000;
  interval_100ms.tv_sec = 0;
  interval_100ms.tv_nsec = 100000000;
  interval_1s.tv_sec = 1;
  interval_1s.tv_nsec = 0;
  interval_1ms.tv_sec = 0;
  interval_1ms.tv_nsec = 1000000;
  interval_10ms.tv_sec = 0;
  interval_10ms.tv_nsec = 10000000;
  
  fd1=open_msr(0);
  fd2=open_msr(8);

  /* Calculate the units used */
  result1=read_msr(fd1,MSR_RAPL_POWER_UNIT);
  
  power_units=pow(0.5,(double)(result1&0xf));
  energy_units=pow(0.5,(double)((result1>>8)&0x1f));
  time_units=pow(0.5,(double)((result1>>16)&0xf));


  uint64_t currentval, newval, mask = 0, offset = 0;
  currentval=read_msr(fd1,MSR_PKG_RAPL_POWER_LIMIT);  
  //printf("RAPL power limit1 = %.6fW\n", power_units*(double)(currentval&0x7fff));
  currentval=read_msr(fd2,MSR_PKG_RAPL_POWER_LIMIT);
  //printf("RAPL power limit2 = %.6fW\n", power_units*(double)(currentval&0x7fff));
  FILE *PowerFilePointer;
  PowerFilePointer = fopen("socket_power.txt","w");
  fclose(PowerFilePointer);

  int i; 
  i = 0;
  while (i<20){
	i++;
 	PowerFilePointer = fopen("socket_power.txt","a");

  	result1=read_msr(fd1,MSR_PKG_ENERGY_STATUS);
        result2=read_msr(fd2,MSR_PKG_ENERGY_STATUS);  
	package1_before=(double)result1*energy_units;
	package2_before=(double)result2*energy_units;
        gettimeofday(&currentime1, NULL);
  	
	nanosleep(&interval_100ms, NULL);

  	result1=read_msr(fd1,MSR_PKG_ENERGY_STATUS); 
        result2=read_msr(fd2,MSR_PKG_ENERGY_STATUS); 
  	gettimeofday(&currentime2, NULL);
  	


	package1_after=(double)result1*energy_units;
 // 	printf("Package1 energy after: %.6f  (%.6fJ consumed)\n",
//	package1_after,package1_after-package1_before);
        
        package2_after=(double)result2*energy_units;
   //     printf("Package2 energy after: %.6f  (%.6fJ consumed)\n",
  //      package2_after,package2_after-package2_before);
 	
	nowtime =((long) ((currentime2.tv_usec - beginningtime.tv_usec) + (currentime2.tv_sec - beginningtime.tv_sec)* 1000000))/1000000.000000;
     //   printf("nowtime =%.6LF\n", nowtime);
  	power1 =((package1_after - package1_before) /((currentime2.tv_usec - currentime1.tv_usec) + (currentime2.tv_sec - currentime1.tv_sec)*1000000))*1000000;
    //    printf("power1 =%LF\n", power1);
  	power2 =((package2_after - package2_before) /((currentime2.tv_usec - currentime1.tv_usec) + (currentime2.tv_sec - currentime1.tv_sec)*1000000))*1000000;
        long double SocketPower = power1 + power2;
	fprintf(PowerFilePointer,"%LF\n",SocketPower);
//        printf("power1 = %lF, power2= %lF, sum = %lF", power1,power2,SocketPower);
	sleep(1);
	fclose(PowerFilePointer);
	}

}
