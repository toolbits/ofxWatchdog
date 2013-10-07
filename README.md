ofxWatchdog
===========

2013/10/07<br/>
Current version is not compatible with Xcode 5.<br/>
When application is starting, SIGTRAP is raised from glfwInit().<br/>
I'm trying to fix this issue.<br/>
<br/>
<br/>
A process watchdog timer for openFrameworks applications.<br/>
You can watch your application's hang-up, illegal memory access, zero devide.<br/>
Then exit safely or reboot application.<br/>
<br/>
Only two functions:<br/>
<br/>
  ofxWatchdog::watch(int msec, bool reboot, bool verbose)<br/>
  ofxWatchdog::clear(void)<br/>
