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

class pythermalzone:

    temperaturefile=None
    
    def __init__ (self):
        """ """

        if os.path.isfile ("/proc/acpi/thermal_zone/THRM/temperature"):
            self.temperaturefile="/proc/acpi/thermal_zone/THRM/temperature"
            return
            
        if os.path.isfile ("/proc/acpi/thermal_zone/THM0/temperature"):
            self.temperaturefile="/proc/acpi/thermal_zone/THM0/temperature"
            return
            
        if os.path.isfile ("/proc/acpi/thermal_zone/THR1/temperature"):
            self.temperaturefile="/proc/acpi/thermal_zone/THR1/temperature"
            return
        
        try:
            zones=os.listdir ("/proc/acpi/thermal_zone")
        except:
            raise errors.ERROR_NO_INTERFACE
            
        if len (zones) == 0:
            raise errors.ERROR_NO_INTERFACE
            
        for zone in zones:
            if os.path.isfile ("/proc/acpi/thermal_zone/"+zone+"/temperature"):
                self.temperaturefile="/proc/acpi/thermal_zone/"+zone+"/temperature"
                return
                
        raise errors.ERROR_NO_INTERFACE
            
    def temperature (self):
        """Get CPU's temperature using Kernel's ACPI Thermal Zone Driver if available"""
        
        if self.temperaturefile == None:
            return None
            
        if not os.path.isfile (self.temperaturefile):
            return None
        try:
            f=file(self.temperaturefile,"r")
        except:
            return None
        tmp=string.rstrip(f.readline())
        f.close()
        if tmp == "<not supported>":
            return None
        tmp=string.split(tmp,":")
        temp=string.rstrip(tmp[1])
        return string.lstrip(temp)
        
def GetTemperature ():
    try:
        tz=pythermalzone()
    except errors.ERROR_NO_INTERFACE:
        # no thermal_zone support
        return None
    return tz.temperature()
