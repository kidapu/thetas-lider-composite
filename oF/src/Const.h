//
// Const.h
//

#pragma once

#pragma mark - enums

enum DEBUG_MODES {
    MODE_LIVE = 0, // live mode
    MODE_DEBUG = 1 // debug mode
};


#pragma mark - class

class Const
{
public:
    //デバッグモード
    const static int DEBUG_MODE = MODE_DEBUG;
    
    
    // for app
    const static int IMG_SIZE = 225; //225;

    // for ofxLider
    const static int FRAME_MAX = 4;
    const static int PACKET_LEN = 1206;
    const static int DATA_BLOCK_NUM = 12;
    const static int LASER_NUM = 16;
    const static int COORD_MAX = 90000;
    const static int TMP_COORD_MAX = 10;
    
    // udpManagerå
    const static int UDP_PORT = 2368;
    const static int PACKETS_MAX_LEN = 50; // 50パケット溜まったら、一気に捨てる
    
    // oscManager
    const static int NODE_PORT = 6000;
};


