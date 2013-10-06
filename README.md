ofxWatchdog
===========

A process watchdog timer for openFrameworks applications.
You can watch your application's hang-up, illegal memory access, zero devide.
Then exit safely or reboot application.

Only two functions:

  ofxWatchdog::watch(int msec, bool reboot, bool verbose)
  ofxWatchdog::clear(void)
