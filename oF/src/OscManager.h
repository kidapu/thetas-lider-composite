//
//  Osc Manager
//

#pragma once

#include "ofxOsc.h"


class OscManager
{
public:

    // vars
    ofxOscSender _oscSender;
    
    void setup()
    {
        _oscSender.setup("127.0.0.1", Const::NODE_PORT);
    }
    
//    void sendFolderPath(string folder)
//    {
//        ofxOscMessage m;
//        m.setAddress("/folder");
//        m.addStringArg(folder);
//        _oscSender.sendMessage(m);
//    }
    
    void sendImagePath(string imgPath)
    {
        ofxOscMessage m;
        m.setAddress("/file");
        m.addStringArg(imgPath);
        _oscSender.sendMessage(m);
    }
};

