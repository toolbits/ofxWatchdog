#include "ofMain.h"
#include "testApp.h"
#include "ofxWatchdog.h"

//========================================================================
int main(){
    // ofxWatchdog::watch(msec, reboot, override, verbose)
    //
    //     msec : how long does the watchdog wait, when the process hangs-up in milli seconds
    //   reboot : automatically restart the process
    // override : use internal signal handler
    //  verbose : print more log information
    
    if (ofxWatchdog::watch(3000, false, true, true)) {
        ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context
        
        // this kicks off the running of my app
        // can be OF_WINDOW or OF_FULLSCREEN
        // pass in width and height too:
        ofRunApp( new testApp());
    }
}
