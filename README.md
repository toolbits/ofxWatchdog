ofxWatchdog version 2
===========

A process watchdog timer for openFrameworks applications.<br/>
You can watch your application's hang-up, illegal memory access,<br/>
illegal instruction, segmentation fault, zero devide, abort (uncaught C++ exception).<br/>
Then exit safely or reboot application.<br/>
<br/>
If you are using openFrameworks 0.9.4 or older, please use <a href="https://github.com/toolbits/ofxWatchdog/tree/version1_final">version1_final</a> tag version instead.<br/>
<br/>
<br/>
<br/>
Only two functions to use:<br/>
<br/>
o ofxWatchdog::boot(int msec, bool reboot, bool override, bool verbose)<br/>
o ofxWatchdog::trap(void)<br/>
<br/>
Optionally you can use:<br/>
<br/>
o ofxWatchdog::clear(void)<br/>
<br/>
<br/>
<br/>
Development environment:<br/>
[version 2]<br/>
&lt;NEW!> o MacOS X 10.14.2 + Xcode 10.1 + of 0.10.1 osx<br/>
o MacOS X 10.14 + Xcode 10.0 + of_v20181009_osx_nightly<br/>

[version 1]<br/>
o MacOS X 10.12.6 + Xcode 9.0 + of 0.9.4 osx<br/>
o MacOS X 10.11.3 + Xcode 6.2 + of 0.9.2 osx<br/>
o MacOS X 10.10.3 + Xcode 6.3.1 + of 0.8.4 osx<br/>
o ubuntu linux 14.04 LTS + Code::Blocks 13.12 + of 0.8.4 linux 64<br/>
o MacOS X 10.10 + Xcode 6.1 + of 0.8.4 osx<br/>
o MacOS X 10.9.4 + Xcode 5.1.1 + of 0.8.3 osx<br/>
o MacOS X 10.8.5 + Xcode 5.0 + of 0.8.0 osx<br/>
o MacOS X 10.7.5 + Xcode 4.6.2 + of 0.8.0 osx<br/>
