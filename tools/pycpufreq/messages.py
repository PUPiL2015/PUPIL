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

# cpuspeedy localized messages
EN={\
                "MSG_INVALID_OPT":"invalid option",
                "MSG_INVALID_SPEED_UNIT":"invalid speed unit given",
                "MSG_INVALID_SPEED_VALUE":"invalid speed value given",
                "MSG_NO_PRIVILEGE":"you must be root to change the CPU speed.",
                "MSG_CPU_SPEED":"CPU speed is",
                "MSG_CPU_SPEED_NOW":"CPU speed is now",
                "MSG_CPU_TEMPERATURE":"temperature is",
                "MSG_NO_FREQS":"the frequencies list is not available.",
                "MSG_NO_ARGS":"no arguments given",
                "MSG_SPEED_UNIT_HELPER":
                    "Argument 2 must be a valid frequency unit: KHz,MHz or GHz.\n",
                "MSG_SPEED_VALUE_HELPER":
                    "The speed value must be expressed in Khz, Mhz o Ghz.\n"\
                    +"eg: 1.0 Ghz or 800 Mhz\n",
                "MSG_INTERFACE_HELPER":
                    "\nIf you are running a v2.5/2.6 kernel, please make sure that:\n"\
                    +"  - That you have the core cpufreq and cpufreq-userspace modules\n"\
                    +"    compiled or loaded into the kernel.\n"\
                    +"  - The you have sysfs mounted /sys\n"\
                    +"  - That you have the cpufreq driver for your cpu loaded.\n"\
                    +"If you are running a v2.4 kernel, please make sure that:\n"\
                    +"...\n"\
                    +"If you still have problems please email the author:\n"\
                    "Gabriele Giorgetti <gabriele_giorgetti@tin.it>\n"            
}
IT={\
                "MSG_INVALID_OPT":"opzione non valida",
                "MSG_INVALID_SPEED_UNIT":"unita' di velocita' non valida",
                "MSG_INVALID_SPEED_VALUE":"valore di velocita' non valido",
                "MSG_NO_PRIVILEGE":"e' necessario essere root per cambiare la velocita' della CPU.",
                "MSG_CPU_SPEED":"la velocita' della CPU e'",
                "MSG_CPU_SPEED_NOW":"la velocita' della CPU ora e'",
                "MSG_CPU_TEMPERATURE":"la temperatura e' di",
                "MSG_NO_FREQS":"la lista delle frequenze non e' disponibile.",
                "MSG_NO_ARGS":"nessun argomento passato",
                "MSG_SPEED_UNIT_HELPER":
                    "L'argomento 2 deve essere un unita' di frequenza: KHz,MHz o GHz.\n",
                "MSG_SPEED_VALUE_HELPER":
                    "Il valore di velocita' deve essere espresso in Khz, Mhz o Ghz.\n"\
                    +"es: 1.0 Ghz o 800 Mhz\n",
                "MSG_INTERFACE_HELPER":
                    "\nSe si sta usando un kernel versione 2.5/2.6, assicurarsi che:\n"\
                    +"  - I moduli cpufreq e cpufreq-userspace siano presenti\n"\
                    +"    compilati o caricati nel kernel.\n"\
                    +"  - Che il filesystem sysfs sia montato in /sys\n"\
                    +"  - Che un driver cpufreq per la propria cpu sia caricato.\n"\
                    +"Se si sta usando un kernel versione 2.4 assicurarsi che:\n"\
                    +"...\n"\
                    +"Se si hanno ancora problemi con il programma, contattare l'autore:\n"\
                    "Gabriele Giorgetti <gabriele_giorgetti@tin.it>\n"    
}
