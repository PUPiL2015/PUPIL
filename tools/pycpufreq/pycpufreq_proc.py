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

import os,string
import errors

class proc:
    """ """
    cpu=None
    speed=None
    speed_min=None
    speed_max=None
    
    def __init__(self,cpu=0):
        """ """
        self.cpu=cpu
        if not os.path.isfile("/proc/cpufreq"):
            raise errors.ERROR_INVALID_INTERFACE
        
    def status(self):
        """ """ 
        f=file("/proc/cpufreq","r")
        # header: minimum CPU frequency  os.getuid()os.getuid()-  maximum CPU frequency  -  policy
        line=f.readline()
        # FIXME: first CPU only !
        cpu0line=string.split(f.readline()," ")
        n=0
        done=-1
        for i in cpu0line:
            if i=="kHz" and done==-1:
                self.speed_min=int(cpu0line[n-1])
                done=0
                n=n+1
                continue
            elif i=="kHz" and done==0:
                self.speed_max=int(cpu0line[n-1])
                break
            n=n+1
        return True
        
    def setspeed (self,frequency):
        """ """
        min=frequency-(int(self.cpuinfo_min_freq)\
            +(int(self.cpuinfo_max_freq)-int(self.cpuinfo_max_freq))/2)
        max=frequency+(int(self.cpuinfo_min_freq)\
            +(int(self.cpuinfo_max_freq)-int(self.cpuinfo_max_freq))/2)
        if min <= self.cpuinfo_min_freq:
            min = self.cpuinfo_min_freq
        if max >= self.cpuinfo_max_freq:
            max = self.cpuinfo_max_freq
            
        f=file("/proc/cpufreq","w")
        f.write(str(self.cpu)+":"+str(min)+":"+str(max)+":"+"userspace")
        f.close()
        
        self.status()
        return True
        
    def dump(self):
        sys.stdout.write("minimum CPU frequency : "+str(self.speed_min)+"\n")
        sys.stdout.write("maximum CPU frequency: "+str(self.speed_max)+"\n")
        sys.stdout.write("policy:")
