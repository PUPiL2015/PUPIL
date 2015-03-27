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
import errors, pythermalzone

# Try to guess the the best available interface 
GUESS=0
# SYSFS CPUFreq interface (kernel v2.5/v2.6)
SYSFS=1
# Kernel 2.4 CPUFreq interface (deprecated in kernel v2.6)
API24=2
# Proc CPUFreq interface (deprecated)
PROC=3

def _osinfo():
    pipe=os.popen("uname -sro")
    osinfo=pipe.read()
    pipe.close()
    return string.rstrip(osinfo)

class pycpufreq:
    # CPU 
    cpu=None
    # interfaces are: GUESS,SYSFS,API24,PROC
    interface=None
    # descriptive name for the interface
    interface_name=None
    # interface handler
    handler=None
    # frequency values
    speed=None
    speed_min=None
    speed_max=None

    def __init__ (self,cpu=0,interface=GUESS):
        """ """
        self.cpu=cpu
        
        if interface == GUESS:
            if self.has_interface(SYSFS):
                self.interface=SYSFS
                self.interface_name="SYSFS"
            elif self.has_interface(API24):
                self.interface=API24
                self.interface_name="API24"
            elif self.has_interface(PROC):
                self.interface=PROC
                self.interface_name="PROC"
            else:
                raise errors.ERROR_NO_INTERFACE
        else:
            self.interface=interface

        if self.interface == SYSFS:
            import pycpufreq_sysfs
            try:
                self.handler=pycpufreq_sysfs.sysfs(cpu=cpu)
            except errors.ERROR_SYSFS_NO_USERSPACE:
                # the userspace governor, which is needed by cpuspeedy in order
                # to change the CPU frequency, is not available. 
                raise errors.ERROR_SYSFS_NO_USERSPACE
                
        elif self.interface == API24:
            import pycpufreq_api24
            self.handler=pycpufreq_api24.api24(cpu=cpu)
        elif self.interface == PROC:
            import pycpufreq_proc
            self.handler=pycpufreq_proc.proc(cpu=cpu)
        else:
            raise errors.ERROR_INVALID_INTERFACE
            
    def has_interface(self,interface):
        """ """
        if interface == SYSFS:
            if os.path.isfile("/sys/devices/system/cpu/"+"cpu"+str(self.cpu)\
                +"/cpufreq/scaling_available_governors"):
                return True
        elif interface == API24:
            if os.path.isfile("/proc/sys/cpu/"+str(self.cpu)+"/speed"):
                return True
        elif interface == PROC:
            if os.path.isfile("/proc/cpufreq"):
                return True
        return False
        
    def freqs (self):
        """ """
        if self.interface!=SYSFS:
            return None
        return self.handler.scaling_available_frequencies
        
    def status (self):
        """ """
        self.handler.status()
        if self.handler.speed==None:
            self.speed=int(self.handler.speed_max)
        else:
            self.speed=int(self.handler.speed)
        self.speed_min=int(self.handler.speed_min)
        self.speed_max=int(self.handler.speed_max)

    def setspeed (self,frequency):
        """ """
        self.handler.setspeed(frequency)
        self.speed=int(self.handler.speed)
        self.status()
        return True
        
    def dump(self):
        sys.stdout.write("%s %s\n" %("os:",str(_osinfo())))
        sys.stdout.write("%s %s\n" %("cpu:",str(self.cpu)))
        sys.stdout.write("%s %s (%s)\n" %("interface:",str(self.interface_name),str(self.interface)))
        sys.stdout.write("%s %s\n" %("speed:",str(self.speed)))
        sys.stdout.write("%s %s\n" %("speed_min:",str(self.speed_min)))
        sys.stdout.write("%s %s\n" %("speed_max:",str(self.speed_max)))
        temperature=pythermalzone.GetTemperature()
        if temperature!=None:
            sys.stdout.write("%s %s\n" %("temperature:",str(temperature)))
        sys.stdout.write ("-\n")
        self.handler.dump()
