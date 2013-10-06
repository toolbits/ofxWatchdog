ofxWatchdog
===========

A process watchdog timer for openFrameworks applications.</br>
You can watch your application's hang-up, illegal memory access, zero devide.</br>
Then exit safely or reboot application.</br>
</br>
Only two functions:</br>
</br>
  ofxWatchdog::watch(int msec, bool reboot, bool verbose)</br>
  ofxWatchdog::clear(void)</br>
