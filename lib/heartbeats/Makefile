CXX = /usr/bin/gcc
CXXFLAGS = -Wall -Wno-unknown-pragmas -Iinc -Llib -O6
DBG = -g
DEFINES ?= 
#LDFLAGS = -lpthread -lrt -lhb-file -lhrm-file
LDFLAGS = -lpthread -lhb-shared -lhrm-shared -lrt

DOCDIR = doc
BINDIR = bin
LIBDIR = lib
INCDIR = ./inc
SCRATCH = ./scratch
OUTPUT = ./output
SRCDIR = ./src
ROOTS = application system tp lat core-allocator
TEST_ROOTS = test1 test2
BINS = $(ROOTS:%=$(BINDIR)/%)
TESTS = $(TEST_ROOTS:%=$(BINDIR)/%)
OBJS = $(ROOTS:%=$(BINDIR)/%.o)
TEST_OBJS = $(TEST_ROOTS:%=$(BINDIR)/%.o)

all: $(BINDIR) $(LIBDIR) $(SCRATCH) shared $(OUTPUT) $(BINS) shared-accuracy

$(BINDIR):
	-mkdir -p $(BINDIR)

$(LIBDIR):
	-mkdir -p $(LIBDIR)

$(SCRATCH):
	-mkdir -p $(SCRATCH)

$(OUTPUT):
	-mkdir -p $(OUTPUT)


$(BINDIR)/%.o : $(SRCDIR)/%.c
	$(CXX) -c $(CXXFLAGS) $(DEFINES) $(DBG) -o $@ $<


$(BINS) : $(OBJS) 

$(BINS) : % : %.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(TESTS) : $(TEST_OBJS) 

$(TESTS) : % : %.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)


bench-tp:
	$(MAKE) clean
	$(MAKE) all DEFINES="-DSHARED"
	./bin/tp 1000 "" > $(OUTPUT)/tp_shmem_based.out
	cat $(OUTPUT)/tp_shmem_based.out
	#$(MAKE) clean
	#$(MAKE) all DEFINES="-DFILEBASED"
	#./bin/tp 1000 $(OUTPUT)/tp_file_based.log > $(OUTPUT)/tp_file_based.out
	#cat $(OUTPUT)/tp_file_based.out

bench-lat:
	$(MAKE) clean
	$(MAKE) all
	ls $(SCRATCH) | $(BINDIR)/lat 1000 $(OUTPUT)/log > $(OUTPUT)/lat_shmem_based.out
	cat $(OUTPUT)/lat_shmem_based.out

#test:
#	$(MAKE) clean
#	$(MAKE) $(BINDIR) $(SCRATCH) $(OUTPUT) $(BINS) $(TESTS)

# Heartbeat shared memory version
shared: $(LIBDIR)/libhb-shared.a $(LIBDIR)/libhrm-shared.a

shared-accuracy: $(LIBDIR)/libhb-acc-shared.a $(LIBDIR)/libhrm-acc-shared.a

$(LIBDIR)/libhb-shared.a: $(SRCDIR)/heartbeat-shared.c $(INCDIR)/heartbeat.h
	$(MAKE) $(BINDIR)/heartbeat-shared.o
	ar r $(LIBDIR)/libhb-shared.a $(BINDIR)/heartbeat-shared.o
	ranlib $(LIBDIR)/libhb-shared.a

$(LIBDIR)/libhb-acc-shared.a: $(SRCDIR)/heartbeat-accuracy-shared.c $(INCDIR)/heartbeat-accuracy.h
	$(MAKE) $(BINDIR)/heartbeat-accuracy-shared.o
	ar r $(LIBDIR)/libhb-acc-shared.a $(BINDIR)/heartbeat-accuracy-shared.o
	ranlib $(LIBDIR)/libhb-acc-shared.a

$(LIBDIR)/libhb-acc-pow-shared.a: $(SRCDIR)/heartbeat-accuracy-power-shared.c $(INCDIR)/heartbeat-accuracy-power.h
	$(MAKE) $(BINDIR)/heartbeat-accuracy-power-shared.o
	ar r $(LIBDIR)/libhb-acc-pow-shared.a $(BINDIR)/heartbeat-accuracy-power-shared.o
	ranlib $(LIBDIR)/libhb-acc-pow-shared.a

$(LIBDIR)/libhb-acc-pow-shared2.a: $(SRCDIR)/heartbeat-accuracy-power-shared2.c $(INCDIR)/heartbeat-accuracy-power.h
	$(MAKE) $(BINDIR)/heartbeat-accuracy-power-shared2.o
	ar r $(LIBDIR)/libhb-acc-pow-shared2.a $(BINDIR)/heartbeat-accuracy-power-shared2.o
	ranlib $(LIBDIR)/libhb-acc-pow-shared2.a

$(LIBDIR)/libhrm-shared.a: $(SRCDIR)/heart_rate_monitor-shared.c $(INCDIR)/heart_rate_monitor.h
	$(MAKE) $(BINDIR)/heart_rate_monitor-shared.o
	ar r $(LIBDIR)/libhrm-shared.a $(BINDIR)/heart_rate_monitor-shared.o
	ranlib $(LIBDIR)/libhrm-shared.a

$(LIBDIR)/libhrm-acc-shared.a: $(SRCDIR)/heart_rate_monitor-accuracy-shared.c $(INCDIR)/heart_rate_monitor-accuracy.h
	$(MAKE) $(BINDIR)/heart_rate_monitor-accuracy-shared.o
	ar r $(LIBDIR)/libhrm-acc-shared.a $(BINDIR)/heart_rate_monitor-accuracy-shared.o
	ranlib $(LIBDIR)/libhrm-acc-shared.a

# Heartbeat file version
filebased: $(LIBDIR)/libhb-file.a $(LIBDIR)/libhrm-file.a

$(LIBDIR)/libhb-file.a: $(SRCDIR)/heartbeat-file.c $(INCDIR)/heartbeat.h
	$(MAKE) $(BINDIR)/heartbeat-file.o
	ar r $(LIBDIR)/libhb-file.a $(BINDIR)/heartbeat-file.o
	ranlib $(LIBDIR)/libhb-file.a

$(LIBDIR)/libhrm-file.a: $(SRCDIR)/heart_rate_monitor-file.c $(INCDIR)/heart_rate_monitor.h
	$(MAKE) $(BINDIR)/heart_rate_monitor-file.o
	ar r $(LIBDIR)/libhrm-file.a $(BINDIR)/heart_rate_monitor-file.o
	ranlib $(LIBDIR)/libhrm-file.a

## cleaning
clean:
	-rm -rf $(BINDIR) $(SCRATCH) *.log *~ $(SRCDIR)/*~

squeaky: clean
	-rm -rf $(OUTPUT)

# Documentation
documentation: 
	doxygen heartbeats_doc

clean-documentation:
	-rm -rf $(DOCDIR)/html/*
	-rm -rf $(DOCDIR)/latex/*
