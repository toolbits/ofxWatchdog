#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // To use ofxWatchdog addon, you have to
    // insert some codes into main() function.
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(ofRandom(255), ofRandom(255), ofRandom(255));
}

//--------------------------------------------------------------
#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma clang optimize off
void ofApp::keyPressed(int key) throw (std::exception) {
    int volatile z = 1;
    char* volatile p = 0;
    
    switch (key) {
        case 'i':
        case 'I':
            // illegal instruction
            raise(SIGILL);
            break;
        case 'a':
        case 'A':
            // abort
            abort();
            break;
        case 'e':
        case 'E':
            // exception
            throw std::exception();
            break;
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
        case 's':
        case 'S':
            // segmentation fault
            (*((void(*)(int))&p))(z);
            break;
        case 'h':
        case 'H':
            // infinite loop
            while (true);
            break;
    }
}
#pragma clang optimize on
#pragma GCC pop_options

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
