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


#pragma once


#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "IPVideoGrabber.h"
#include "ofxOsc.h"


#define NUM_CAMERAS 1
#define NUM_ROWS 1
#define NUM_COLS 1

// listen on port 7400
#define PORT 7400
#define NUM_MSG_STRINGS 20


class IPCameraDef
{
public:
    IPCameraDef()
    {
    }

    IPCameraDef(const std::string& url): _url(url)
    {
    }

    IPCameraDef(const std::string& name,
                const std::string& url,
                const std::string& username,
                const std::string& password):
        _name(name),
        _url(url),
        _username(username),
        _password(password)
    {
    }

    void setName(const std::string& name) { _name = name; }
    std::string getName() const { return _name; }

    void setURL(const std::string& url) { _url = url; }
    std::string getURL() const { return _url; }

    void setUsername(const std::string& username) { _username = username; }
    std::string getUsername() const { return _username; }

    void setPassword(const std::string& password) { _password = password; }
    std::string getPassword() const { return _password; }


private:
    std::string _name;
    std::string _url;
    std::string _username;
    std::string _password;
};


using ofx::Video::IPVideoGrabber;
using ofx::Video::SharedIPVideoGrabber;

class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

    std::vector<SharedIPVideoGrabber> grabbers;

    void loadCameras();
    IPCameraDef& getNextCamera();
    std::vector<IPCameraDef> ipcams; // a list of IPCameras
    int nextCamera;

    // This message occurs when the incoming video stream image size changes.
    // This can happen if the IPCamera has a single broadcast state (some cheaper IPCams do this)
    // and that broadcast size is changed by another user.

    void videoResized(const void* sender, ofResizeEventArgs& arg);

    //serial
    void mousePressed(int x, int y, int button);

    void moveForwardLeft();
    void moveForwardRight();
    void moveForward();
    void moveBackwards();
    void moveBackwardsRight();
    void moveBackwardsLeft();
    void moveRight();
    void moveLeft();

    bool		bSendSerialMessage;			// a flag for sending serial
    char		bytesRead[3];				// data from serial, we will be trying to read 3
    char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
    int			nBytesRead;					// how much did we read?
    int			nTimesRead;					// how many times did we read?
    float		readTime;					// when did we last read?

    ofSerial	serial;
    ofxOscReceiver receiver;

    int current_msg_string;
    string msg_strings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];

    float mouseX, mouseY;
    string mouseButtonState;
};





