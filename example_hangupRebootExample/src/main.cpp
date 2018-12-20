#include "ofMain.h"
#include "ofApp.h"
#include "ofxWatchdog.h"

//========================================================================
int main( ){
    // ofxWatchdog::boot(msec, reboot, override, verbose)
    //
    //     msec : how long does the watchdog wait, when the process hangs-up in milli seconds
    //   reboot : automatically restart the process
    // override : use internal signal handler (optional)
    //  verbose : print more log information (optional)
    ofxWatchdog::boot(3000, true, true, true);
    
    ofSetupOpenGL(1024, 768, OF_WINDOW);            // <-------- setup the GL context
    
    // ofxWatchdog::trap(void)
    ofxWatchdog::trap();
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
    
}
