// =============================================================================
//
// Copyright (c) 2009-2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"


//------------------------------------------------------------------------------
void ofApp::setup()
{
    cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup(PORT);

    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(30);
    loadCameras();

    // initialize connection
    for(std::size_t i = 0; i < NUM_CAMERAS; i++)
    {
        IPCameraDef& cam = getNextCamera();

        SharedIPVideoGrabber c = IPVideoGrabber::makeShared();

        // if your camera uses standard web-based authentication, use this
        // c->setUsername(cam.username);
        // c->setPassword(cam.password);

        // if your camera uses cookies for authentication, use something like this:
        // c->setCookie("user", cam.username);
        // c->setCookie("password", cam.password);

        c->setCameraName(cam.getName());
        c->setURI(cam.getURL());
        c->connect(); // connect immediately

        // if desired, set up a video resize listener
        ofAddListener(c->videoResized, this, &ofApp::videoResized);

        grabbers.push_back(c);

    }
    	ofSetVerticalSync(true);

	bSendSerialMessage = false;
	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();

	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....
	int baud = 9600;
	//serial.setup(0, baud); //open the first device
	serial.setup("COM3", baud); // windows example
	//serial.setup("/dev/tty.usbserial-A4001JEC", baud); // mac osx example
	//serial.setup("/dev/ttyUSB0", baud); //linux example

	nTimesRead = 0;
	nBytesRead = 0;
	readTime = 0;
	memset(bytesReadString, 0, 4);
}

//------------------------------------------------------------------------------
IPCameraDef& ofApp::getNextCamera()
{
    nextCamera = (nextCamera + 1) % ipcams.size();
    return ipcams[nextCamera];
}

//------------------------------------------------------------------------------
void ofApp::loadCameras()
{

    // all of these cameras were found using this google query
    // http://www.google.com/search?q=inurl%3A%22axis-cgi%2Fmjpg%22
    // some of the cameras below may no longer be valid.

    // to define a camera with a username / password
    //ipcams.push_back(IPCameraDef("http://148.61.142.228/axis-cgi/mjpg/video.cgi", "username", "password"));

	ofLog(OF_LOG_NOTICE, "---------------Loading Streams---------------");
    ipcams.push_back(IPCameraDef("http://192.168.0.5:8080/video")); // home
    //ipcams.push_back(IPCameraDef("http://10.170.168.100:8080/video")); // uni
//	ofxXmlSettings XML;
//
//	if(XML.loadFile("streams.xml"))
//    {
//
//        XML.pushTag("streams");
//        std::string tag = "stream";
//
//		int nCams = XML.getNumTags(tag);
//
//		for(std::size_t n = 0; n < nCams; n++)
//        {
//
//            IPCameraDef def(XML.getAttribute(tag, "name", "", n),
//                            XML.getAttribute(tag, "url", "", n),
//                            XML.getAttribute(tag, "username", "", n),
//                            XML.getAttribute(tag, "password", "", n));
//
//
//            std::string logMessage = "STREAM LOADED: " + def.getName() +
//			" url: " +  def.getURL() +
//			" username: " + def.getUsername() +
//			" password: " + def.getPassword();
//
//            ofLogNotice() << logMessage;
//
//            ipcams.push_back(def);
//
//		}
//
//		XML.popTag();
//
//	}
//    else
//    {
//		ofLog(OF_LOG_ERROR, "Unable to load streams.xml.");
//	}
//
    ofLog(OF_LOG_NOTICE, "-----------Loading Streams Complete----------");
//
//    nextCamera = ipcams.size();
}

//------------------------------------------------------------------------------
void ofApp::videoResized(const void* sender, ofResizeEventArgs& arg)
{
    // find the camera that sent the resize event changed
    for(std::size_t i = 0; i < NUM_CAMERAS; i++)
    {
        if(sender == &grabbers[i])
        {
            std::stringstream ss;
            ss << "videoResized: ";
            ss << "Camera connected to: " << grabbers[i]->getURI() + " ";
            ss << "New DIM = " << arg.width << "/" << arg.height;
            ofLogVerbose("ofApp") << ss.str();
        }
    }
}


//------------------------------------------------------------------------------
void ofApp::update()
{
    // update the cameras
    for(std::size_t i = 0; i < grabbers.size(); i++)
    {
        grabbers[i]->update();
    }

    //serial
    if (bSendSerialMessage){

		// (1) write the letter "a" to serial:
		serial.writeByte('a');

		// (2) read
		// now we try to read 3 bytes
		// since we might not get them all the time 3 - but sometimes 0, 6, or something else,
		// we will try to read three bytes, as much as we can
		// otherwise, we may have a "lag" if we don't read fast enough
		// or just read three every time. now, we will be sure to
		// read as much as we can in groups of three...

		nTimesRead = 0;
		nBytesRead = 0;
		int nRead  = 0;  // a temp variable to keep count per read

		unsigned char bytesReturned[3];

		memset(bytesReadString, 0, 4);
		memset(bytesReturned, 0, 3);

		while( (nRead = serial.readBytes( bytesReturned, 3)) > 0){
			nTimesRead++;
			nBytesRead = nRead;
		};

		memcpy(bytesReadString, bytesReturned, 3);

		bSendSerialMessage = false;
		readTime = ofGetElapsedTimef();
	}

    //osc
    while(receiver.hasWaitingMessages()){
        // get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);
        if(m.getAddress() == "/COG/LEFT"){
			// both the arguments are int32's
			mouseX = m.getArgAsFloat(0);
			moveForwardLeft();
        }else if(m.getAddress() == "/COG/RIGHT"){
          	mouseX = m.getArgAsFloat(0);
			moveForwardRight();
        }else if(m.getAddress() == "/COG/PUSH"){
          	mouseX = m.getArgAsFloat(0);
          	if(m.getArgAsFloat(0) > 0.8){
                moveForward();
            }
        }else if(m.getAddress() == "/COG/PULL"){
          	mouseX = m.getArgAsFloat(0);
			moveBackwards();
        }else{
			// unrecognized message: display on the bottom of the screen
//			string msg_string;
//			msg_string = m.getAddress();
//			msg_string += ": ";
//			for(int i = 0; i < m.getNumArgs(); i++){
//				// get the argument type
//				msg_string += m.getArgTypeName(i);
//				msg_string += ":";
//				// display the argument - make sure we get the right type
//				if(m.getArgType(i) == OFXOSC_TYPE_INT32){
//					msg_string += ofToString(m.getArgAsInt32(i));
//				}
//				else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
//					msg_string += ofToString(m.getArgAsFloat(i));
//				}
//				else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
//					msg_string += m.getArgAsString(i);
//				}
//				else{
//					msg_string += "unknown";
//				}
//			}
//			// add to the list of strings to display
//			msg_strings[current_msg_string] = msg_string;
//			timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
//			current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
//			// clear the next line
//			msg_strings[current_msg_string] = "";
		}
    }

}

//------------------------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0,0,0);

	ofSetHexColor(0xffffff);

    int row = 0;
    int col = 0;

    int x = 0;
    int y = 0;

    int w = ofGetWidth() / NUM_COLS;
    int h = ofGetHeight() / NUM_ROWS;

    float totalKbps = 0;
    float totalFPS = 0;

    for(std::size_t i = 0; i < grabbers.size(); i++)
    {
        x = col * w;
        y = row * h;

        // draw in a grid
        row = (row + 1) % NUM_ROWS;

        if(row == 0)
        {
            col = (col + 1) % NUM_COLS;
        }


        ofPushMatrix();
        ofTranslate(x,y);
        ofSetColor(255,255,255,255);
        grabbers[i]->draw(0,0,w,h); // draw the camera

        ofEnableAlphaBlending();

        float kbps = grabbers[i]->getBitRate() / 1000.0f; // kilobits / second, not kibibits / second
        totalKbps+=kbps;

        float fps = grabbers[i]->getFrameRate();
        totalFPS+=fps;


        ofPopMatrix();
    }

    // keep track of some totals
    float avgFPS = totalFPS / NUM_CAMERAS;
    float avgKbps = totalKbps / NUM_CAMERAS;

    ofEnableAlphaBlending();
    ofSetColor(255);
    // ofToString formatting available in 0072+
    ofDrawBitmapString(" AVG FPS: " + ofToString(avgFPS,2/*,7,' '*/), 10,17);
    ofDrawBitmapString("AVG Kb/S: " + ofToString(avgKbps,2/*,7,' '*/), 10,29);
    ofDrawBitmapString("TOT Kb/S: " + ofToString(totalKbps,2/*,7,' '*/), 10,41);

    string buf;
	buf = "listening for osc messages on port" + ofToString(PORT);
	ofDrawBitmapString(buf, 10, 53);

    	// draw mouse state
	buf = "Command: " + ofToString(mouseX, 4) +  " " + ofToString(mouseY, 4);
	ofDrawBitmapString(buf, 430, 20);
	ofDrawBitmapString(mouseButtonState, 580, 20);

	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		ofDrawBitmapString(msg_strings[i], 10, 40 + 15 * i);
	}


    ofDisableAlphaBlending();

    ofLine(w/3, 0, w/3, h);
    ofLine(2*w/3, 0, 2*w/3, h);
    ofLine(0, h/3, w, h/3);
    ofLine(0, 2*h/3, w, 2*h/3);


}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    //camera refresh
    if(key == ' ')
    {
        // initialize connection
        for(std::size_t i = 0; i < NUM_CAMERAS; i++)
        {
            ofRemoveListener(grabbers[i]->videoResized, this, &ofApp::videoResized);
            SharedIPVideoGrabber c = IPVideoGrabber::makeShared();
            IPCameraDef& cam = getNextCamera();
            c->setUsername(cam.getUsername());
            c->setPassword(cam.getPassword());
            Poco::URI uri(cam.getURL());
            c->setURI(uri);
            c->connect();

            grabbers[i] = c;
        }
    }

}

void ofApp::mousePressed(int x, int y, int button){
    int w = ofGetWidth() / NUM_COLS;
    int h = ofGetHeight() / NUM_ROWS;
    //top row
    //left
    if(x > 0 && x < w/3 && y < h/3 && y > 0){
        moveForwardLeft();
    //middle
    } else if(x > w/3 && x < 2*(w/3) && y < h/3 && y > 0){
        moveForward();
    //right
    } else if(x > 2*(w/3) && x < w && y < h/3 && y > 0){
        moveForwardRight();

    //middle row
    //left
    } else if(x > 0 && x < (w/3) && y < 2*(h/3) && y > (h/3) ){
        moveLeft();
    //middle
    } else if(x > w/3 && x < 2*(w/3) && y < 2*(h/3) && y > (h/3) ){
        serial.writeByte('s');
    //right1
    } else if (x > 2*(w/3) && x < w && y < 2*(h/3) && y > (h/3)){
        moveRight();

   //botton row
    //left
    } else if(x > 0 && x < w/3 && y > 2*(h/3) && y < h ){
        moveBackwardsLeft();
    //middle
    } else if(x > w/3 && x < 2*(w/3) && y > 2*(h/3) && y < h){
        moveBackwards();
    //right
    } else if(x > 2*(w/3) && x < w && y > 2*(h/3) && y < h ){
        moveBackwardsRight();
    }
}

void gotMessage(ofMessage msg);
void ofApp::moveForwardLeft(){
    serial.writeByte('q');
}
void ofApp::moveForwardRight(){
    serial.writeByte('e');
}
void ofApp::moveForward(){
    serial.writeByte('w');
}
void ofApp::moveBackwards(){
    serial.writeByte('x');
}
void ofApp::moveBackwardsRight(){
    serial.writeByte('c');
}
void ofApp::moveBackwardsLeft(){
    serial.writeByte('z');
}
void ofApp::moveRight(){
    serial.writeByte('d');
}
void ofApp::moveLeft(){
    serial.writeByte('a');
}
