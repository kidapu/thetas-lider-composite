//
//  UdpManager.h
//

#pragma once


// addon
#include "ofxNetwork.h"


#pragma mark - structs

// パケットデータ
struct PacketChar
{
    char c[Const::PACKET_LEN];
};

// 頂点データ
struct Coord
{
    ofVec3f position;
    int intensity;
    float azimuth;
    int laserId;
    int zindex;
    int dist;
};


#pragma mark - typedef

typedef vector<Coord> CoordVec; //1次元配列(azimuth順)にCoord(頂点データ)をぶっこむコンテナ
typedef vector< vector< Coord > > CoordVec2; //2次元配列(zindex,azimuth順)にCoord(頂点データ)をぶっこむコンテナ


class UdpManager : protected ofThread
{
public:
    
    // vars
    ofxUDPManager _udp;
    
    // data container
    vector<PacketChar> _datas;
    
    // constructer
    ~UdpManager() {};
    
    void setup()
    {
        // udp
        _udp.Create();
        _udp.Bind(Const::UDP_PORT);
        _udp.SetNonBlocking(true);
        
        // thread
        stopThread();
        startThread(true);
    }

    void exit()
    {
        stopThread();
    }

    
    # pragma mark - accsessor
    
    PacketChar extractData()
    {
        PacketChar res;
        
        // 
        int len = _datas.size();
        if( len == 0 )
        {
            return;
        }
        
        //
        lock();
        {
            res = _datas[0];
            _datas.erase( _datas.begin() );
        }
        unlock();

        
        // あまり溜まり過ぎるようなら、捨てる
        if( Const::PACKETS_MAX_LEN < len )
        {
            cout << "delete recieve data" << endl;
            _datas.clear();
        }
        
        return res;
    }
    
    
    # pragma mark - delegate methods
    
    // recieve thread
    void threadedFunction()
    {
        
        while( isThreadRunning() )
        {
            
            // udp recieve
            char udpMessage[Const::PACKET_LEN];
            int udpState = _udp.Receive(udpMessage, Const::PACKET_LEN);
            if( udpState == SOCKET_ERROR || udpState == INVALID_SOCKET)
            {
                continue;
            }
            
            //
            PacketChar pc;
            for(int i = 0; i < Const::PACKET_LEN; i++)
            {
                pc.c[i] = udpMessage[i];
            }
            
            lock();
            {
                _datas.push_back( pc );
            }
            unlock();


//            float time = clock() / 1000000.0;
//        　  cout << time << " / " <<  _datas.size() << endl;
        
            // dumpUdpData( udpMessage );
        }
    }
    

private :
    
    # pragma mark - parse methods

    float dumpUdpData(const char * udpMessage)
    {
        for(int i = 0; i < Const::PACKET_LEN; i++)
        {
            printf("%i/%02x ", i, (unsigned char)udpMessage[i]);
        }
        cout << "" << endl;
        cout << "--------" << endl;
    }
    
    
};