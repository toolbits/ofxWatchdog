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
<br/>
<br/>
Major modification supporting of 0.9.2<br/>
<br/>
NEW:<br/>
int main(){<br/>
&nbsp;&nbsp;&nbsp;&nbsp;ofSetupOpenGL(1024,768, OF_WINDOW);<br/>
<br/>
&nbsp;&nbsp;&nbsp;&nbsp;ofxWatchdog::watch(3000, true, true, true);<br/>
&nbsp;&nbsp;&nbsp;&nbsp;…<br/>
}<br/>
<br/>
OLD:<br/>
int main(){<br/>
&nbsp;&nbsp;&nbsp;&nbsp;ofxWatchdog::watch(3000, true, true, true);<br/>
<br/>
&nbsp;&nbsp;&nbsp;&nbsp;ofSetupOpenGL(1024,768, OF_WINDOW);<br/>
&nbsp;&nbsp;&nbsp;&nbsp;…<br/>
}<br/>
<br/>
Development environment:<br/>
o &lt;NEW!> MacOS X 10.11.3 + Xcode 6.2 + of 0.9.2 osx<br/>
o MacOS X 10.10.3 + Xcode 6.3.1 + of 0.8.4 osx<br/>
o ubuntu linux 14.04 LTS + Code::Blocks 13.12 + of 0.8.4 linux 64<br/>
o MacOS X 10.10 + Xcode 6.1 + of 0.8.4 osx<br/>
o MacOS X 10.9.4 + Xcode 5.1.1 + of 0.8.3 osx<br/>
o MacOS X 10.8.5 + Xcode 5.0 + of 0.8.0 osx<br/>
o MacOS X 10.7.5 + Xcode 4.6.2 + of 0.8.0 osx<br/>
