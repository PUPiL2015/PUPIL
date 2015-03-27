#!/usr/bin/env python

# cpuspeedy
# http://cpuspeedy.sourceforge.net
#
# Copyright (C) Gabriele Giorgetti <gabriele_giorgetti@tin.it>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os,sys,string
# The install prefix will be added to the sys.path by make according to the Makefile.
sys.path=[os.curdir]+sys.path
import pycpufreq,pythermalzone,messages,errors

PACKAGE="cpuspeedy"
VERSION="0.4.1"
RELEASE_DATE="20040702"
AUTHOR="Gabriele Giorgetti"
EMAIL="<gabriele_giorgetti@tin.it>"
URL="http://cpuspeedy.sourceforge.net"

# exit values
EXIT_SUCCESS=0
EXIT_FAILURE=1

EXIT_ERR_INTERFACE_NONE=2
EXIT_ERR_INTERFACE_INVALID=3

EXIT_ERR_INVALID_SPEED_UNIT=4
EXIT_ERR_INVALID_SPEED_VALUE=5

EXIT_ERR_SETSPEED_MIN=10
EXIT_ERR_SETSPEED_LOW=11
EXIT_ERR_SETSPEED_MED=12
EXIT_ERR_SETSPEED_HIGH=13
EXIT_ERR_SETSPEED_MAX=14
EXIT_ERR_SETSPEED_VAL=15

# Options
OPT_FREQS=("-f", "--freqs")
OPT_STATUS=("-s","--status")
OPT_DUMP=("-d","--dump")
OPT_HELP=("-h","--help")
OPT_VERSION=("-v","--version")
# hidden option to be used with the gtk fronted
OPT_FRONTEND=("--frontend")

# Speed values
OPT_SPEED_MIN=("min","minimum")
OPT_SPEED_LOW=("low","slow")
OPT_SPEED_MED=("med","medium")
OPT_SPEED_HIGH=("high","fast")
OPT_SPEED_MAX=("max","maximum")

# Speed units
OPT_SPEED_KHZ="KHz"
OPT_SPEED_MHZ="Mhz"
OPT_SPEED_GHZ="GHz"

# localized messages dictionary
MSG={}
    
def SPEED_MIN (min):
    """ """
    return int(min)

def SPEED_LOW (min,max):
    """ """
    return int(int(min)+(int(max)-int(min))/4)
    
def SPEED_MED (min,max):
    """ """
    return int(int(min)+(int(max)-int(min))/2)
    
def SPEED_HIGH (min,max):
    """ """
    return int(int(max)-(int(max)-int(min))/4)
    
def SPEED_MAX (max):
    """ """
    return int(max)
    
def SPEED_HUMAN (frequency):
    """converts a frequency value from KHz to MHz or Ghz"""
    divisor=None
    units=None
    if int(frequency) > 999999: # kHz
        divisor = 1000 * 1000
        units = OPT_SPEED_GHZ
    else:
        divisor = 1000
        units = OPT_SPEED_MHZ
    if ((int(frequency) % divisor) == 0) or divisor == 1000: #integer
        return str("%d %s" %((int(frequency) / divisor),units))
    else: # float
        return str("%3.2f %s" %((float(frequency) / divisor),units))

def SPEED_MACHINE (frequency, unit):
    """converts a frequency value from Ghz or Mhz to KHz"""
    if string.lower(unit) == string.lower(OPT_SPEED_GHZ):
        multiplyer=1000 * 1000
    elif string.lower(unit) == string.lower(OPT_SPEED_MHZ):
        multiplyer=1000
    elif string.lower(unit) == string.lower(OPT_SPEED_KHZ):
        multiplyer=1
    else:
        return None
    try:
        float(frequency)
    except:
        # Invalid frequency
        return None
    return str(int(float(frequency) * int(multiplyer)))

def SPEED_PERCENT (fmin,fmax):
    """Returns the CPU speed as percent value"""
    return str("%d%%" % ((fmin * 100) / fmax))

def show_version():
    sys.stdout.write ("%s %s (%s) (%s)\n" %(PACKAGE,VERSION,RELEASE_DATE,URL))
    sys.stdout.write ("\n")
    sys.stdout.write ("Written by %s %s\n" %(AUTHOR,EMAIL))
    
def show_help():
    sys.stdout.write ("%s version %s Usage: %s [SPEED VALUE] or [OPTION]\n" %(PACKAGE,VERSION,PACKAGE))
    sys.stdout.write ("\n")
    sys.stdout.write ("Example: %s 1.07 Ghz\n" %(PACKAGE))
    sys.stdout.write ("Example: %s 800 Mhz\n" %(PACKAGE))
    sys.stdout.write ("Example: %s low\n" %(PACKAGE))
    sys.stdout.write ("\n")
    sys.stdout.write ("Speed values:\n")
    sys.stdout.write ("      [numeric value] [unit] (unit must be: Khz,MHz or Ghz)\n" )
    sys.stdout.write ("      %s, %s\n" %(OPT_SPEED_MIN[0],OPT_SPEED_MIN[1]))
    sys.stdout.write ("      %s, %s\n" %(OPT_SPEED_LOW[0],OPT_SPEED_LOW[1]))
    sys.stdout.write ("      %s, %s\n" %(OPT_SPEED_MED[0],OPT_SPEED_MED[1]))
    sys.stdout.write ("      %s, %s\n" %(OPT_SPEED_HIGH[0],OPT_SPEED_HIGH[1]))
    sys.stdout.write ("      %s, %s\n" %(OPT_SPEED_MAX[0],OPT_SPEED_MAX[1]))
    sys.stdout.write ("\n")
    sys.stdout.write ("Options:\n")
    sys.stdout.write ("      %s, %s            print infos about the CPU speed and temperature\n" %(OPT_STATUS[0],OPT_STATUS[1]))
    sys.stdout.write ("      %s, %s             print a list of available frequencies (2.6.X only).\n" %(OPT_FREQS[0],OPT_FREQS[1]))
    sys.stdout.write ("      %s, %s              dump infos about available interface if any\n" %(OPT_DUMP[0],OPT_DUMP[1]))
    sys.stdout.write ("      %s, %s              display this help and exit\n" %(OPT_HELP[0],OPT_HELP[1]))
    sys.stdout.write ("      %s, %s           output version information and exit\n" %(OPT_VERSION[0],OPT_VERSION[1]))
    sys.stdout.write ("\n")    
    sys.stdout.write ("Report bugs to %s.\n" %(EMAIL))
    
def main():
    """ """
    # set the language dictionary
    global MSG
    lang=os.getenv("LANG")
    if lang == "it_IT":
        MSG=messages.IT
    else:
        MSG=messages.EN
    
    # no argument given    
    if len(sys.argv)==1:
        sys.stderr.write(PACKAGE+": "+MSG["MSG_NO_ARGS"]+"\n")
        sys.stderr.write("Try `"+PACKAGE+" "+OPT_HELP[1]+"' for more information.\n")
        sys.exit (EXIT_FAILURE)
        
    speed_unit=None
    # two or more arguments have been given
    # in this case argument 2 must be a speed unit string (khz,mhz or ghz)
    if len(sys.argv)>=3:
        # A specific speed value (in KHZ,MHZ,GHZ) has been requested 
        if string.lower(sys.argv[2]) in (string.lower(OPT_SPEED_KHZ),\
            string.lower(OPT_SPEED_MHZ),string.lower(OPT_SPEED_GHZ)):
            speed_unit=sys.argv[2]
        else:
            sys.stderr.write(PACKAGE+": "+MSG["MSG_INVALID_SPEED_UNIT"]+ " " +sys.argv[2]+"\n")
            sys.stdout.write(MSG["MSG_SPEED_UNIT_HELPER"])
            sys.exit(EXIT_ERR_INVALID_SPEED_UNIT)       
            
    request=sys.argv[1]
        
    if request in OPT_HELP:
        show_help()
        sys.exit(EXIT_SUCCESS)
        
    if request in OPT_VERSION:
        show_version()
        sys.exit(EXIT_SUCCESS)
        
    # initialize the pycpufreq class
    try:
        cpufreq=pycpufreq.pycpufreq()
    except errors.ERROR_NO_INTERFACE:
        # FIXME: try to be more descriptive here 
        sys.stderr.write(PACKAGE+": "+"error: ERROR_NO_INTERFACE"+"\n")
        sys.stdout.write(MSG["MSG_INTERFACE_HELPER"])
        sys.exit(EXIT_ERR_INTERFACE_NONE)
    except errors.ERROR_INVALID_INTERFACE:
        # FIXME: try to be more descriptive here too
        sys.stderr.write(PACKAGE+": "+"error: ERROR_INVALID_INTERFACE"+"\n")
        sys.stdout.write(MSG["MSG_INTERFACE_HELPER"])
        sys.exit(EXIT_ERR_INTERFACE_INVALID)
        
    cpufreq.status()
    
    if request in OPT_FREQS:
        freqs=cpufreq.freqs()
        if freqs==None:
            sys.stdout.write (PACKAGE+": "+MSG["MSG_NO_FREQS"]+"\n")
            sys.exit(EXIT_SUCCESS)
        for freq in freqs:
            sys.stdout.write (freq+" KHz ("+SPEED_HUMAN (freq)+")\n")
        sys.exit(EXIT_SUCCESS)
        
    if request in OPT_STATUS:
        sys.stdout.write (PACKAGE+": "+MSG["MSG_CPU_SPEED"]+" "\
                + SPEED_HUMAN(cpufreq.speed)+" ("\
                + SPEED_PERCENT(cpufreq.speed,cpufreq.speed_max)+")")
        temperature=pythermalzone.GetTemperature()
        if temperature!=None:
            sys.stdout.write (" "+MSG["MSG_CPU_TEMPERATURE"]+" "+temperature)
        sys.stdout.write ("\n")
        sys.exit(EXIT_SUCCESS)
    
    if request in OPT_DUMP:
        sys.stdout.write ("%s %s\n" % (str(PACKAGE+":"),str(VERSION)))
        sys.stdout.write ("-\n")
        cpufreq.dump()
        sys.exit(EXIT_SUCCESS)
        
    if request in OPT_FRONTEND:
        sys.stdout.write (SPEED_HUMAN(cpufreq.speed)+"|"+SPEED_PERCENT(cpufreq.speed,cpufreq.speed_max)+"|")
        temperature=pythermalzone.GetTemperature()
        if temperature!=None:
            sys.stdout.write (temperature+"|")
        sys.stdout.write ("\n")
        sys.exit(EXIT_SUCCESS)        
        
    # OK at this point 'request' is not an option but probably a speed value
    speed_value=request
    
    if string.lower(speed_value) in OPT_SPEED_MIN\
        or string.lower(speed_value) in OPT_SPEED_LOW\
        or string.lower(speed_value) in OPT_SPEED_MED\
        or string.lower(speed_value) in OPT_SPEED_HIGH\
        or string.lower(speed_value) in OPT_SPEED_MAX\
        or speed_unit != None:
        if os.getuid() != 0:
            sys.stderr.write(PACKAGE+": "+MSG["MSG_NO_PRIVILEGE"]+"\n")
            sys.exit(EXIT_FAILURE)
            
    if string.lower(speed_value) in OPT_SPEED_MIN:
        if not cpufreq.setspeed(SPEED_MIN(cpufreq.speed_min)):
            sys.exit(EXIT_ERR_SETSPEED_MIN)
        
    elif string.lower(speed_value) in OPT_SPEED_LOW:
        if not cpufreq.setspeed(SPEED_LOW(cpufreq.speed_min,cpufreq.speed_max)):
            sys.exit(EXIT_ERR_SETSPEED_LOW)
            
    elif string.lower(speed_value) in OPT_SPEED_MED:
        if not cpufreq.setspeed(SPEED_MED(cpufreq.speed_min,cpufreq.speed_max)):
            sys.exit(EXIT_ERR_SETSPEED_MED)
            
    elif string.lower(speed_value) in OPT_SPEED_HIGH:
        if not cpufreq.setspeed(SPEED_HIGH(cpufreq.speed_min,cpufreq.speed_max)):
            sys.exit(EXIT_ERR_SETSPEED_HIGH)
            
    elif string.lower(speed_value) in OPT_SPEED_MAX:
        if not cpufreq.setspeed (SPEED_MAX(cpufreq.speed_max)):
            sys.exit(EXIT_ERR_SETSPEED_MAX)
            
    # if 'speed_unit' is set, a specific speed value (in KHz,MHz, or GHz) has been given
    elif speed_unit != None:
        if SPEED_MACHINE(speed_value,speed_unit)==None:
            # oops, an invalid (non numeric) speed_value has been given
            sys.stderr.write(PACKAGE+": "+MSG["MSG_INVALID_SPEED_VALUE"]+ " " +str(speed_value)+"\n")
            sys.stdout.write(MSG["MSG_SPEED_VALUE_HELPER"])
            sys.exit(EXIT_ERR_INVALID_SPEED_VALUE)  
            
        if not cpufreq.setspeed (SPEED_MACHINE(speed_value,speed_unit)):
            sys.exit(EXIT_ERR_SETSPEED_VAL)
            
    else:
        sys.stderr.write(PACKAGE+": "+MSG["MSG_INVALID_OPT"]+" "+request+"\n")
        sys.stderr.write("Try `"+PACKAGE+" "+OPT_HELP[1]+"' for more information.\n")
        sys.exit(EXIT_FAILURE)
    
    sys.stdout.write (PACKAGE+": "+MSG["MSG_CPU_SPEED_NOW"]+" "\
            + SPEED_HUMAN(cpufreq.speed)+" ("\
            + SPEED_PERCENT(cpufreq.speed,cpufreq.speed_max)+")")
    temperature=pythermalzone.GetTemperature()
    if temperature!=None:
        sys.stdout.write (" "+MSG["MSG_CPU_TEMPERATURE"]+" "+temperature)
    sys.stdout.write ("\n")

if __name__ == "__main__":
    main()
    sys.exit(EXIT_SUCCESS)
