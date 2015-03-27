Project Description
----------------------------------------
The Performance Under Power Limit (PUPiL) is a software(decision tree)/hardware(RAPL)
hybrid power limiting approach. It maxes the performance while strictly respecting power
cap during application runtime.


Running PUPiL Examples
----------------------------------------
First ensure that you have installed the Heartbeats library.

Second, RAPL stuff.... 

Build the library and test binaries:

$ make

Run the tests:

$ export LD_LIBRARY_PATH="./lib"
$ ./bin/[test_binary] [num_beats] [target_rate] [window_size]

The num_beats is the number of total heartbeats the application will cycle
through before exiting. The target_rate is the number of heartbeats per second,
and the window size is the number of heartbeats used to calculate the heartbeat
rate. It doesn't make sense to give the test binary a target heartbeat rate
without knowing the normal heartbeat rate of the application on your system. To
find this, run:

$ ./bin/[test_binary] [num_beats] 0 [window_size]

If you aren't sure what parameters to use, a good place to start is 200 beats
and a window size of 20.

IMPORTANT NOTE: The tests will not run until the environment variable
HEARTBEAT_ENABLED_DIR is set, which is required by the Heartbeats library.
Just set it to a temporary directory:

$ export HEARTBEAT_ENABLED_DIR=/tmp


Installing/Uninstalling POET
----------------------------------------
To install PUPiL to the local system, run:

$ make install

Headers are installed to /usr/local/include/poet.
The library is installed to /usr/local/lib.

To uninstall PUPiL, run:

$ make uninstall


Directory Structure
----------------------------------------
.
├── config    -- Default and example configuration files 
├── inc       -- Header files 
├── src       -- Source files 
└── test      -- Test source files 
