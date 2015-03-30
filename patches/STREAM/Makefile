CC = gcc
CFLAGS = -O3 -I/home/hankhoffmann/heartbeats-development/heartbeats-google-code/heartbeats/inc/ -L/home/hankhoffmann/heartbeats-development/heartbeats-google-code/heartbeats/lib

FF = g77
FFLAGS = -O3

all: stream_c.exe

#stream_f.exe: stream.f mysecond.o
#	$(CC) $(CFLAGS) -c mysecond.c
#	$(FF) $(FFLAGS) -c stream.f
#	$(FF) $(FFLAGS) stream.o mysecond.o -o stream_f.exe

stream_c.exe: stream.c
	$(CC) $(CFLAGS) -fopenmp stream.c -o stream_c.exe -lhb-acc-pow-shared -lm

clean:
	rm -f stream_f.exe stream_c.exe *.o
