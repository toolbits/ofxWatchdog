#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    // To use ofxWatchdog addon, you have to add
    // some codes into main() function and testApp::update().
}

//--------------------------------------------------------------
void testApp::update(){
    ofxWatchdog::clear();
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(ofRandom(255), ofRandom(255), ofRandom(255));
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    int z = 1;
    char* p = 0;
    
    switch (key) {
        case 'z':
        case 'Z':
            // zero devide
            z /= 0;
            break;
        case 'b':
        case 'B':
            // illegal access
            *p = 0xAB;
            break;
        case 'h':
        case 'H':
            // infinite loop
            while (true);
            break;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}