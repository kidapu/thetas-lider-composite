//
//  ofxLider.h
//

#pragma once

#include "PacketManager.h"


class ofxLider : protected ofThread
{
public:
    
    // vars
    PacketManager _packetManager;

    // data container
    vector< CoordVec2 > _frames;
    
    // constructer
    ~ofxLider() {};

    void setup()
    {
        _packetManager.setup();
        
        // thread
        stopThread();
        startThread(true);
    }
    
    void exit()
    {
        stopThread();
        _packetManager.exit();
    }
    
    void extractData(CoordVec2 & frame)
    {
        int len = _frames.size();
        if(len == 0 )
        {
            return;
        }
        
        lock();
        {
            frame = _frames[0];
            _frames.erase(_frames.begin());
        }
        unlock();
        
        
        // あまり溜まり過ぎるようなら、捨てる
        if( Const::FRAME_MAX < len )
        {
            cout << "delete frame" << endl;
            _frames.clear();
        }
    }

    
    # pragma mark - delegate methods
    
    // parse thread
    void threadedFunction()
    {
        while( isThreadRunning() )
        {
            CoordVec coordVec;
            _packetManager.extractData(coordVec);
            
            // null
            int size = coordVec.size();
            if( 0 == size )
            {
                continue;
            }
            
            
            // zidexに整理する ( CoordVec -> CoordVec2 変換 )
            CoordVec2 frame;
            frame.resize( Const::LASER_NUM );
            for( int i = 0; i < size ; i+=1 )
            {
                frame[ coordVec[i].zindex ].push_back( coordVec[i] );
            }
            
            // zindexごとの数を確認
//            for( int i = 0; i < Const::LASER_NUM ; i++ )
//            {
//                cout << i << " : " << frame[i].size() << endl;
//            }
            
            
            // save
            lock();
            {
                _frames.push_back(frame);
            }
            unlock();
            
        }
    }
    

};
