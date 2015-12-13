//
//  PacketManager.h
//

#pragma once

// my lib
#include "UdpManager.h"


class PacketManager : protected ofThread
{
public:
    // vars
    UdpManager _udpManager;
    
    // container
    CoordVec _datas;
    CoordVec _tmpDatas;
    
    // const contaier
    map<int, int> _laserAngles;
    map<int, int> _laserZindex;
    map<int, string> _returnModes;
    map<int, string> _sensorTypes;

    void setup()
    {
        // const map arrays
        _laserAngles[0] = -15; //0
        _laserAngles[2] = -13; //1
        _laserAngles[4] = -11; //2
        _laserAngles[6] = -9; //3
        _laserAngles[8] = -7; //4
        _laserAngles[10] = -5; //5
        _laserAngles[12] = -3; //6
        _laserAngles[14] = -1; //7
        _laserAngles[1] = 1; //8
        _laserAngles[3] = 3; //9
        _laserAngles[5] = 5; //10
        _laserAngles[7] = 7; //11
        _laserAngles[9] = 9; //12
        _laserAngles[11] = 11; //13
        _laserAngles[13] = 13; //14
        _laserAngles[15] = 15; //15

        _laserZindex[0] = 0;
        _laserZindex[2] = 1;
        _laserZindex[4] = 2;
        _laserZindex[6] = 3;
        _laserZindex[8] = 4;
        _laserZindex[10] = 5;
        _laserZindex[12] = 6;
        _laserZindex[14] = 7;
        _laserZindex[1] = 8;
        _laserZindex[3] = 9;
        _laserZindex[5] = 10;
        _laserZindex[7] = 11;
        _laserZindex[9] = 12;
        _laserZindex[11] = 13;
        _laserZindex[13] = 14;
        _laserZindex[15] = 15;
        
        _returnModes[37] = "Strongest Return";
        _returnModes[38] = "Last Return";
        _returnModes[39] = "Dual Return";
        
        _sensorTypes[21] = "HDL-32E";
        _sensorTypes[22] = "VLP-16";
        
        
        // data container
        _datas.resize(Const::COORD_MAX);
        _tmpDatas.resize(Const::TMP_COORD_MAX);
        
        
        // thread
        stopThread();
        startThread(true);
        
        
        // setup
        _udpManager.setup();
    }
    
    
    void exit()
    {
        stopThread();
        _udpManager.exit();
    }
    
    
    void extractData(CoordVec & frame)
    {
        // 360 -> 0への切り替わりを探す
        int len = _datas.size();
        int right = 0;
        int interval = 5000;
        for( int i = interval; i < len - interval ; i+=interval )
        {
            if(_datas[i].azimuth < _datas[i-interval].azimuth)
            {
                right = i;
                break;
            }
        }
        
        
        // まだデータがすくない or 同じデータが溜まり続けてる
        int left = right - interval;
        if(left <= 0 )
        {
            // 溜まり過ぎてたら捨てる
            deleteExtraDatas();
            return;
        }
        
        
        // 2分探索して、360->0へ切り替わるindexを探す
        int lastIdx = 0;
        float threadAzimuth = 0.0;
        while( true )
        {
            int mid = (left+right)/2;
            
            float azimuth0 = ( threadAzimuth < _datas[left].azimuth) ? threadAzimuth + 360.0 : threadAzimuth;
            float midVal = ( _datas[mid].azimuth < _datas[left].azimuth) ? _datas[mid].azimuth + 360.0 : _datas[mid].azimuth;
            float midVal2 = ( _datas[mid+1].azimuth < _datas[left].azimuth) ? _datas[mid+1].azimuth + 360.0 : _datas[mid+1].azimuth;
            

            //
            if((midVal <= azimuth0)
            && (azimuth0 <= midVal2) )
            {
                lastIdx = mid;
                break;
            }
            else if( midVal < azimuth0 )
            {
                left = mid;
            }
            else if( azimuth0 <= midVal)
            {
                right = mid;
            }
        }


        
        // 0 - lastIdxを1フレームとしてぶっこぬく
        lock();
        {
            copy(_datas.begin(), _datas.begin() + lastIdx,  back_inserter(frame) );
            _datas.erase(_datas.begin(), _datas.begin() + lastIdx);
        }
        unlock();
    
        
        // さらに過剰にデータが溜まり過ぎてたら捨てる。(あんまりここに入ってほしくない...。)
        deleteExtraDatas();
        
        return;
        
    }
    
    
    
    // parse thread
    void threadedFunction()
    {
        while( isThreadRunning() )
        {
            
            //// clock
            //clock_t clockStart, clockEnd;
            //clockStart = clock();
            
            // ----- //
            // data check
            // ----- //
            PacketChar packetChar = _udpManager.extractData();
            if( sizeof(packetChar.c) == 0 )
            {
                continue;
            }
            
            // かくにんよう
            //            cout << "確認:" << bin2Hex(result.c_str()[0]) << " : " << result.length() << endl;
            //            continue;
            
            
            // ----- //
            // azimuth
            // ----- //
            // parse azimuth
            map<int, float> azimuthes;
            for(int dataBlockId = 0; dataBlockId < Const::DATA_BLOCK_NUM; dataBlockId++ )
            {
                azimuthes[dataBlockId*2] = parseAzimuth( packetChar.c, dataBlockId );
                // cout << "azi::" << azimuthes[dataBlockId*2] << endl;
            }
            
            // interpolate azimuth
            for(int dataBlockId = 0; dataBlockId < Const::DATA_BLOCK_NUM; dataBlockId++ )
            {
                if( dataBlockId == Const::DATA_BLOCK_NUM - 1 )
                {
                    azimuthes[dataBlockId*2 + 1] = azimuthes[dataBlockId*2] + (azimuthes[dataBlockId*2] - azimuthes[(dataBlockId-1)*2]) * 0.5 ;
                }
                else
                {
                    azimuthes[dataBlockId*2 + 1] = (azimuthes[dataBlockId*2] + azimuthes[(dataBlockId+1)*2]) * 0.5 ;
                }
                
                azimuthes[dataBlockId*2 + 1] = (360 < azimuthes[dataBlockId*2 + 1]) ? azimuthes[dataBlockId*2 + 1] - 360 : azimuthes[dataBlockId*2 + 1];
            }
            
            
            // ----- //
            // factory data
            // ----- //
            // factory data
            //            map<string,string> factoryByte = parseFactoryByte( udpMessage );
            //            cout << factoryByte["returnmode"] << " / " << factoryByte["sensortype"] << endl;
            
            
            // ----- //
            // time stamp
            // ----- //
            //            float timestamp = parseTimeStamp( packetChar.c );
            
            
            // ----- //
            // data block
            // ----- //
            for(int dataBlockId = 0; dataBlockId < Const::DATA_BLOCK_NUM; dataBlockId++ )
            {
                // channel data
                for(int laserId = 0; laserId < Const::LASER_NUM; laserId++)
                {
                    // parse
                    float parseData[4];
                    parseChannel(parseData, packetChar.c, dataBlockId, laserId); // parseData 0:dist1 / 1:intensity1 / 2:dist2 / 3:intensity2
                    
                    // create Coord 1
                    Coord p;
                    p.position = getPosition(laserId, azimuthes[dataBlockId*2], parseData[0] * 100.0);
                    p.intensity = parseData[1];
                    p.azimuth = azimuthes[dataBlockId*2];
                    p.zindex = _laserZindex[laserId];
                    p.dist = parseData[0] * 100.0;
                    
                    // create Coord 2
                    Coord p2;
                    p2.position = getPosition(laserId, azimuthes[dataBlockId*2+1], parseData[2] * 100.0);
                    p2.intensity = parseData[3];
                    p2.azimuth = azimuthes[dataBlockId*2+1];
                    p2.zindex = _laserZindex[laserId];
                    p2.dist = parseData[2] * 100.0;
                    
                    // add
                    //if( 0.0 < parseData[0] )
                    {
                        _tmpDatas.push_back( p );
                    }
                    //if( 0.0 < parseData[2] )
                    {
                        _tmpDatas.push_back( p2 );
                    }
                    
                    if( Const::TMP_COORD_MAX <= _tmpDatas.size() )
                    {
                        // save to common-memory
                        lock();
                        {
                            for( int i = 0 ; i < Const::TMP_COORD_MAX; i++ )
                            {
                                _datas.push_back(_tmpDatas[i]);
                            }
                        }
                        unlock();
                        _tmpDatas.clear();
                    }
                }
            }
            
            //clockEnd = clock();
            //cout << "data block = " << (double)(clockEnd - clockStart) / CLOCKS_PER_SEC << "sec.\n";
            //cout << "-------" << endl;
            
        }
    }
    
    
#pragma mark - parse methods
    
    
    float parseAzimuth(const char * udpMessage, int dataBlockId )
    {
        //
        int c0 = (unsigned char)udpMessage[dataBlockId*100+2];
        int c1 = (unsigned char)udpMessage[dataBlockId*100+3];
        float azimuth = ((c1 << 8) | c0) / 100.0;
        return azimuth;
    }
    
    
    void parseChannel(float result[], const char * udpMessage, int dataBlockId, int laserId)
    {
        //
        int idx = dataBlockId*100+laserId*3;
        
        // parse dist1
        int c0 = (unsigned char)udpMessage[idx+4];
        int c1 = (unsigned char)udpMessage[idx+5];
        float dist1 = ((c1 << 8) | c0) * 2.0 / 1000.0;
        
        // parse intensity1
        int intensity1 = (unsigned char)udpMessage[idx+6];
        
        // parse dist2
        c0 = (unsigned char)udpMessage[idx+52];
        c1 = (unsigned char)udpMessage[idx+53];
        float dist2 = ((c1 << 8) | c0) * 2.0 / 1000.0;
        
        // parse intensity2
        int intensity2 = (unsigned char)udpMessage[idx+54];
        
        // parse data result
        result[0] = dist1;
        result[1] = intensity1;
        result[2] = dist2;
        result[3] = intensity2;
        return;
    }
    
    ofVec3f getPosition( int laserId, float azimuth, float dist)
    {
        float x = dist * cos( _laserAngles[laserId] * DEG_TO_RAD) * sin( azimuth * DEG_TO_RAD ) ;
        float y = dist * cos( _laserAngles[laserId] * DEG_TO_RAD) * cos( azimuth * DEG_TO_RAD ) ;
        float z = dist * sin( _laserAngles[laserId] * DEG_TO_RAD) ;
        return ofVec3f(x,y,z);
    }
    
    map<string, string> parseFactoryByte(const char * udpMessage)
    {
        // parse
        string hexStr1 = bin2Hex( (unsigned char)udpMessage[1204] );
        string hexStr2 = bin2Hex( (unsigned char)udpMessage[1205] );
        
        // return
        map<string, string> result;
        result["returnmode"] = _returnModes[ ofToInt(hexStr1) ];
        result["sensortype"] = _sensorTypes[ ofToInt(hexStr2) ];
        return result;
    }
    
    float parseTimeStamp(const char * udpMessage )
    {
        unsigned int c1 = (unsigned char)udpMessage[1200];
        unsigned int c2 = (unsigned char)udpMessage[1201];
        unsigned int c3 = (unsigned char)udpMessage[1202];
        unsigned int c4 = (unsigned char)udpMessage[1203];
        float timestamp = ((c4<<24) | (c3<<16) | (c2<<8) | c1) / 100000.0 ;
        return timestamp;
    }
    
    
private:
    
    string bin2Hex(const unsigned char binary)
    {
        char hexChar[2];
        snprintf(hexChar, 3, "%02x", binary);
        return (string)hexChar;
    }
    
    // 余分なデータを捨てる
    void deleteExtraDatas()
    {
        int span = _datas.size() - Const::COORD_MAX;
        if( 0 < span )
        {
            lock();
            {
                _datas.erase(_datas.begin(), _datas.begin() + span);
            }
            unlock();
        }
    }


};