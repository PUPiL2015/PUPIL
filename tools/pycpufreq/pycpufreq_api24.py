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

class api24:
    """ """
    cpu=None
    speed=None
    speed_min=None
    speed_max=None
    
    def __init__ (self,cpu=0):
        """ """
        self.cpu=cpu
        if not os.path.isfile("/proc/sys/cpu/"+str(self.cpu)+"/speed"):
            raise errors.ERROR_INVALID_INTERFACE
    
    def status (self):
        """ """
        # reads speed
        f=file("/proc/sys/cpu/"+str(self.cpu)+"/speed","r")
        self.speed=string.rstrip(f.readline())
        f.close
        # reads speed min
        f=file("/proc/sys/cpu/"+str(self.cpu)+"/speed-min","r")
        self.speed_min=string.rstrip(f.readline())
        f.close
        # reads speed_max
        f=file("/proc/sys/cpu/"+str(self.cpu)+"/speed-max","r")
        self.speed_max=string.rstrip(f.readline())
        f.close
        return True

    def setspeed (self,frequency):
        """ """
        if int(frequency) >= int(self.speed_min) \
            and int(frequency) <= int(self.speed_max):
            try:
                f=file("/proc/sys/cpu/"+str(self.cpu)+"/speed","w")
                f.write(str(frequency))
                f.close()
            except:
                return False
                
        f=file("/proc/sys/cpu/"+str(self.cpu)+"/speed-min","w")
        f.write(str(self.speed_min))
        f.close()        
        
        f=file("/proc/sys/cpu/"+str(self.cpu)+"/speed-max","w")
        f.write(str(self.speed_max))
        f.close()         
        self.status()
        return True
