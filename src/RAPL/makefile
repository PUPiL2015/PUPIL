CC=gcc
CFLAGS= -Wall
LIBS=-lm
SRCS= RaplPowerLimitDisable.c RaplSetPowerSeprate.c RaplPowerMonitor.c
#OBJS= $(SRCS:.c=)
all: RaplPowerLimitDisable RaplSetPowerSeprate RaplPowerMonitor

RaplPowerLimitDisable: RaplPowerLimitDisable.c
	$(CC) $(CFLAGS) -o RaplPowerLimitDisable -c RaplPowerLimitDisable.c $(LIBS)
RaplSetPowerSeprate: RaplSetPowerSeprate.c
	$(CC) $(CFLAGS) -o RaplSetPowerSeprate -c RaplSetPowerSeprate.c $(LIBS)
RaplPowerMonitor: RaplPowerMonitor.c
	$(CC) $(CFLAGS) -o RaplPowerMonitor -c RaplPowerMonitor.c $(LIBS)
clean:
	rm -f RaplPowerMonitor RaplSetPowerSeprate RaplPowerLimitDisable
