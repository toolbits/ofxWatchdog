ofxWatchdog
===========

A process watchdog timer for openFrameworks applications.<br/>
You can watch your application's hang-up, illegal memory access,<br/>
illegal instruction, segmentation fault, zero devide, abort (uncaught C++ exception).<br/>
Then exit safely or reboot application.<br/>
<br/>
Only one function to use:<br/>
<br/>
o ofxWatchdog::watch(int msec, bool reboot, bool override, bool verbose)<br/>
<br/>
Optionally you can use:<br/>
<br/>
o ofxWatchdog::clear(void)<br/>
<br/>
Development environment:<br/>
o &gt;NEW!> ubuntu linux 14.04 LTS + Code::Blocks 13.12 + of 0.8.4 linux 64<br/>
o &gt;NEW!> MacOS X 10.10 + Xcode 6.1 + of 0.8.4 osx<br/>
o MacOS X 10.9.4 + Xcode 5.1.1 + of 0.8.3 osx<br/>
o MacOS X 10.8.5 + Xcode 5.0 + of 0.8.0 osx<br/>
o MacOS X 10.7.5 + Xcode 4.6.2 + of 0.8.0 osx<br/>
