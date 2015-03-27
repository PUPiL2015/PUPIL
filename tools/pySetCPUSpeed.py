#!/usr/bin/env python

__author__ = """ Stelios Sidiroglou-Douskos <stelios@csail.mit.edu> """

import sys, os, signal, time, datetime,subprocess
from optparse import OptionParser
from pycpufreq import pycpufreq,messages,errors,cpuspeedy

def detectCPUs():
    """
     Detects the number of CPUs on a system. Cribbed from pp.
     """
    # Linux, Unix and MacOS:
    if hasattr(os, "sysconf"):
       if os.sysconf_names.has_key("SC_NPROCESSORS_ONLN"):
           # Linux & Unix:
           ncpus = os.sysconf("SC_NPROCESSORS_ONLN")
           if isinstance(ncpus, int) and ncpus > 0:
               return ncpus
       else: # OSX:
           return int(os.popen2("sysctl -n hw.ncpu")[1].read())
    # Windows:
    if os.environ.has_key("NUMBER_OF_PROCESSORS"):
           ncpus = int(os.environ["NUMBER_OF_PROCESSORS"]);
           if ncpus > 0:
               return ncpus
    return 1 # Default


def amIRoot():
    return os.getuid() == 0

def setCPUspeed(options):
    for i in range(0, 8):
        cpufreq = pycpufreq.pycpufreq(cpu=i)
        cpufreq.status()
        #print cpufreq.dump()
       
        if not amIRoot():
            print "[-] You need to be root to change frequency!"
            sys.exit(1)

        if options.cpuspeed == "max":
            print "[+] Setting CPU:%d maximum CPU speed:%s" %(i,cpufreq.speed_max)
            cpufreq.setspeed(cpuspeedy.SPEED_MAX(cpufreq.speed_max))
        elif options.cpuspeed == "min":
            print "[+] Setting CPU:%d minimum CPU speed:%s" %(i,cpufreq.speed_min)
            cpufreq.setspeed(cpuspeedy.SPEED_MIN(cpufreq.speed_min))
            
def parseOptions():
    # parse command line options
    parser = OptionParser(usage = "usage: %prog [options]")
   
    parser.add_option("-s", "--status", dest="status",
                      action="store_true",  
                      help="print current CPU status"
                      )
    
    parser.add_option("-f", "--freqs", dest="freqs",
                      action="store_true",  
                      help="print list of available frequencies"
                      )
    
    parser.add_option("-d", "--dump", dest="dump",
                      action="store_true",  
                      help="Print all available information"
                      )
    
    parser.add_option("-n", "--num-freq", dest="numfreq",
                      action="store_true",  
                      help="Return the number of available frequencies"
                      )
    

    parser.add_option("-i", "--interactive", dest="interactive",
                      action="store_true",  
                      help="Interactively set CPU speeds"
                      )
    
    parser.add_option("-S", "--cpu-state", dest="cpustate",
                      action="store", type="string",
                      help="print current CPU status"
                      )
    

    parser.add_option("-v", "--cpu-vector", dest="vector",
                      action="store", type = "string", default = "none", 
                      help="Specify cpu/speed vector. For example," \
                      "0,0,0,0,0,0,0,0 would set 8 CPUs to highest speed."
                      )
    

    return parser

def setupCPUs():
    cpus    = detectCPUs() 
    cpulist = []
    
    #print "[+] This machine has %d CPUs" %cpus
    for i in range(0,cpus):
        cpufreq = pycpufreq.pycpufreq(cpu = i)
        cpufreq.status()
        cpulist.append(cpufreq)
    
    return cpulist

def printCurrentCPUSpeeds(cpulist):
    print "Current CPU speeds"
    print 80 * '-'
    for cpu in cpulist:
            print cpu.speed
    print 80 * '-'

def getAvailableCPUfreq(cpulist):
    cpuspeeds = []
    # Assuming symmetric CPUs here (i.e. same available freqs across CPUs)
    for f in cpulist[0].freqs():
        cpuspeeds.append(f) 
    return cpuspeeds

def setCPUSpeed(cpulist, speed):
    for cpu in cpulist:
        if not cpu.setspeed(speed):
            print "[-] Could not set speed:%d on CPU:%d" %(speed, cpu.cpu)

def main(user_args=None):
    parser = parseOptions()
    (options, args) = parser.parse_args(args=user_args)
   
    # Setup all cpus on machine
    cpulist = setupCPUs()
       
    if options.vector != "none":
        cpuvector = options.vector.split(",")
        print cpuvector
        
        if len(cpuvector) != len(cpulist):
            print "[-] The vector entered:%d is not equal the number of CPUs on this machine:%d" \
                    %(len(cpuvector), len(cpulist))
        
        freqs = getAvailableCPUfreq(cpulist)

        for cpunum in range(len(cpulist)):
            speed = int (freqs[int(cpuvector[cpunum])])
            print "Setting speed:%d for CPU:%d" %(speed, cpunum)
            cpulist[cpunum].setspeed(speed)
        sys.exit(0)

    if options.interactive:
        printCurrentCPUSpeeds(cpulist)
        print "[!] Available CPU frequencies"
        print 80 * '-'
        freqs = getAvailableCPUfreq(cpulist)
        for i in range(len(freqs)):
            print "%d.\t %s" %(i, freqs[i])
        print 80 * '-'

        input = raw_input("Please select which frequency to switch to: ")
        speed =  int (freqs[int(input)])
        print "You selected: %d" %(speed)
        setCPUSpeed(cpulist, speed)
        print "[+] Successfully set CPU speeds to %d" %speed
        sys.exit(0)
    
    if options.status:
        printCurrentCPUSpeeds(cpulist)
        sys.exit(0)
    
    if options.freqs:
        print "Available CPU frequencies"
        print 80 * '-'
        for f in getAvailableCPUfreq(cpulist):
            print f
        sys.exit(0)
   
    if options.numfreq:
        print len(getAvailableCPUfreq(cpulist)) - 1
        sys.exit(0)

    if options.dump:
        cpulist[0].dump() 
        sys.exit(0)

    if options.cpustate:
        freqs = getAvailableCPUfreq(cpulist)
        speed =  int (freqs[int(options.cpustate)])
        setCPUSpeed(cpulist, speed)
        print speed
        sys.exit(0)
    

    parser.print_help()
if __name__ == "__main__":
    main()
 
