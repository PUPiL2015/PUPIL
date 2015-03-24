#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>
//#include <asm/msr.h>
//gcc -O2 -Wall -o rapl-read rapl-read.c -lm
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



int main(void) {
  int fd;
  long long result;
  double power_units,energy_units,time_units;
  double package_before,package_after;
  double pp0_before,pp0_after;
  double pp1_before=0.0,pp1_after;
  double dram_before=0.0,dram_after;
  double thermal_spec_power,minimum_power,maximum_power,time_window;
  int cpu_model;
  

  double power_target = 500;
  printf("Starting set each package power to %fW\n",power_target);

    fd=open_msr(0);

  /* Calculate the units used */
  result=read_msr(fd,MSR_RAPL_POWER_UNIT);
  
  power_units=pow(0.5,(double)(result&0xf));
 // printf("Power units = %.3fW\n",power_units);
 // printf("\n");
  energy_units=pow(0.5,(double)((result>>8)&0x1f));
 // printf("Energy units = %.8fJ\n",energy_units);
 // printf("\n");
  time_units=pow(0.5,(double)((result>>16)&0xf));
 // printf("Time units = %.8fs, %.8fKs\n",time_units, time_units*1000.0);
 // printf("\n");
  set_power_limit(fd, power_target, power_units);

    fd=open_msr(8);

  /* Calculate the units used */
  result=read_msr(fd,MSR_RAPL_POWER_UNIT);
  
  power_units=pow(0.5,(double)(result&0xf));

  energy_units=pow(0.5,(double)((result>>8)&0x1f));

  time_units=pow(0.5,(double)((result>>16)&0xf));

  set_power_limit(fd, power_target, power_units);
 
 


return 1;

}
