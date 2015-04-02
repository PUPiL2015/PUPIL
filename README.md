Project Description
----------------------------------------
The Performance Under Power Limit (PUPiL) is a software(decision tree)/hardware(RAPL)
hybrid power limiting approach. It maxes the performance while strictly respecting power
cap during application runtime.


Running PUPiL Examples
----------------------------------------
First ensure that you have installed the Heartbeats library.
Check out the README file under the Heartbeats sub-directory for install instructions. 
For all the benckmarks, we have inserted Heartbeats segment code and libraries. It should
build successfully by runnning each Makefile under the benchmark directory.

IMPORTANT NOTE: The heartbeat inserted program will not run until the environment variable
HEARTBEAT_ENABLED_DIR is set, which is required by the Heartbeats library.
Just set it to a temporary directory:
$ export HEARTBEAT_ENABLED_DIR=/tmp

In the benchmark patches, we include scripts to show how to run each of the benchmark.

Second, make sure MSRs are enabled for RAPL and also sudo privilege is required.

Build the RAPL interface tools:
Go under the PUPiL/src/RAPL
$ make
Check out the README file under the RAPL sub-directory for the usage of those tools.



Directory Structure
----------------------------------------
.
├── config    -- Default and example configuration files  
├── inc       -- Header files  
├── src       -- Source files   
└── test      -- Test source files 
