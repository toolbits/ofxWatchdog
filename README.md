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
<br/>
Tested environment:<br/>
o MacOS X 10.7.5 + Xcode 4.6.2 + of 0.8.0 osx
o MacOS X 10.8.5 + Xcode 4.6.0 + of 0.8.0 osx
o (!!!NG!!!) MacOS X 10.8.5 + Xcode 5.0 + of 0.8.0 osx
