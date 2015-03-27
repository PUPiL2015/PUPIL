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
import errors

# SYSFS CPUFreq path prefix
SYSFS_PREFIX="/sys/devices/system/cpu/"

class sysfs:
    """ """
    cpu=None
    speed=None
    speed_min=None
    speed_max=None
    # minimum operating frequency the processor can run at(in kHz)
    cpuinfo_min_freq=None
    # maximum operating frequency the processor can run at(in kHz) 
    cpuinfo_max_freq=None
    # CPUfreq driver used on the CPU
    scaling_driver=None
    # CPUfreq governors available in the kernel
    scaling_available_governors=[]
    # available freqs
    scaling_available_frequencies=[]
    # CPUfreq governor in use 
    scaling_governor=None
    # minimum current "policy limit" (in kHz)
    scaling_min_freq=None
    # maximum current "policy limit" (in kHz)
    scaling_max_freq=None
    # current CPU frequency. (only exists in userspace governors)
    scaling_setspeed=None
        
    def __init__ (self,cpu=0):
        """ """
        self.cpu=cpu
        if not os.path.isfile(SYSFS_PREFIX+"cpu"+str(self.cpu)\
                +"/cpufreq/scaling_available_governors"):
            raise errors.ERROR_INVALID_INTERFACE
            
        # CPUfreq governors list check
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
            +"/cpufreq/scaling_available_governors","r")
        governors=\
            string.split(string.rstrip(f.readline()), " ")
        f.close()
        # This is the governor that is needed by cpuspeedy to be able
        # to change the CPU frequency
        if "userspace" not in governors:
            raise ERROR_SYSFS_NO_USERSPACE

    def status(self):
        """ """
        # reads cpuinfo_min_freq
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)+"/cpufreq/cpuinfo_min_freq","r")
        self.cpuinfo_min_freq=string.rstrip(f.readline())
        f.close
        # reads scaling_max_freq
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)+"/cpufreq/scaling_max_freq","r")
        self.scaling_max_freq=string.rstrip(f.readline())
        f.close
        # reads scaling_min_freq
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)+"/cpufreq/scaling_min_freq","r")
        self.scaling_min_freq=string.rstrip(f.readline())
        f.close()
        # reads cpuinfo_max_freq
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)+"/cpufreq/cpuinfo_max_freq","r")
        self.cpuinfo_max_freq=string.rstrip(f.readline())
        f.close()
        # reads scaling_driver
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)+"/cpufreq/scaling_driver","r")
        self.scaling_driver=string.rstrip(f.readline())
        f.close()
        # reads CPUfreq governors list
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
            +"/cpufreq/scaling_available_governors","r")
        self.scaling_available_governors=\
            string.split(string.rstrip(f.readline()), " ")
        f.close()
        # reads CPUfreq governors list
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
            +"/cpufreq/scaling_available_frequencies","r")
        self.scaling_available_frequencies=\
            string.split(string.rstrip(f.readline()), " ")
        f.close()
        # reads scaling governor
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
            +"/cpufreq/scaling_governor","r")
        self.scaling_governor=string.rstrip(f.readline())
        f.close()
        # reads scaling_setspeed
        try:
            # SCALING_SETSPEED only exists in userspace governors 
            f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
                +"/cpufreq/scaling_setspeed","r")
            self.scaling_setspeed=string.rstrip(f.readline())
            f.close()
        except:
            pass
        self.speed=self.scaling_setspeed
        self.speed_min=self.cpuinfo_min_freq
        self.speed_max=self.cpuinfo_max_freq
        return True

    def setspeed (self,frequency):
        """ """
        # SCALING_SETSPEED only exists with userspace governor set
        if self.scaling_governor != "userspace":
            # if the userspace governor is available then is set
            if "userspace" in self.scaling_available_governors:
                f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
                    +"/cpufreq/scaling_governor","w")
                f.write("userspace")
                f.close()
            else:
                return False
                
        # set the frequency value
        if int(frequency) >= int(self.cpuinfo_min_freq) \
            and int(frequency) <= int(self.cpuinfo_max_freq):
            f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
                +"/cpufreq/scaling_setspeed","w")
            f.write(str(frequency))
            f.close()
            
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
            +"/cpufreq/scaling_min_freq","w")
        f.write(self.cpuinfo_min_freq)
        f.close()
        
        f=file(SYSFS_PREFIX+"cpu"+str(self.cpu)\
            +"/cpufreq/scaling_max_freq","w")
        f.write(self.cpuinfo_max_freq)
        f.close()
        
        self.status()
        return True
        
    def dump (self):
        """ """
        sys.stdout.write("scaling_driver: "+self.scaling_driver+"\n")
        sys.stdout.write("scaling_available_governors: ")
        for sag in self.scaling_available_governors:
            sys.stdout.write(sag+",")
        sys.stdout.write ("\n")
        sys.stdout.write("scaling_governor: "+self.scaling_governor+"\n")
        sys.stdout.write("scaling_available_frequencies: ")
        for sag in self.scaling_available_frequencies:
            sys.stdout.write(sag+",")
        sys.stdout.write ("\n")
        sys.stdout.write("cpuinfo_min_freq: "+self.cpuinfo_min_freq+"\n")
        sys.stdout.write("cpuinfo_max_freq: "+self.cpuinfo_max_freq+"\n")
        sys.stdout.write("scaling_min_freq: "+self.scaling_min_freq+"\n")
        sys.stdout.write("scaling_max_freq: "+self.scaling_max_freq+"\n")
        if self.scaling_setspeed != None:
            sys.stdout.write("scaling_setspeed: "\
                +self.scaling_setspeed+"\n")
        return True
