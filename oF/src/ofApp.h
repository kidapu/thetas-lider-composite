#pragma once

#include "ofMain.h"

// my lib
#include "Const.h"
#include "ofxLider.h"
#include "OscManager.h"
#include "ThetasManager.h"

// addon
#include "ofxGui.h"


class ofApp : public ofBaseApp
{
public:
    

    // vars
    ofxLider _lider;
    ofVboMesh _mesh;
    ofEasyCam _cam;
    ThetasManager _thetasMaganer;
    
    // vars
    int _liderFps;
    
    // vars for UI
    ofxPanel _gui;
    ofParameter <int> _pCoordInterval;
    ofParameter <int> _pDistThread;
    ofParameter <float> _pTexWidthRate;
    ofParameter <float> _pTexHeightRate;
    ofParameter <float> _pTexHeightRate2;
    ofxToggle _pToggleMesh;
    
    
    void setup()
    {
        // setting
        ofSetFrameRate( 60.0 );
        
        // lider
        _lider.setup();
        
        // camera
        _cam.setPosition(0, 0, 50);
        _cam.lookAt(ofVec3f(0,0,0), ofVec3f(0,-1,0));
        _cam.enableMouseInput();
        
        // gui
        _gui.setup("params", "guiSettings.xml", 10, 80 );
        _gui.add( _pCoordInterval.set("coordInterval", 7, 1, 50) );
        _gui.add( _pDistThread.set("distThread", 120, 0, 1000) );
        _gui.add( _pTexWidthRate.set("texWidthRate", 0.5, 0.0, 1.0) );
        _gui.add( _pTexHeightRate.set("texHeightRate", 0.2, 0.0, 1.0) );
        _gui.add( _pTexHeightRate2.set("texHeightRate", 0.0, -1.0, 1.0) );
        _gui.add( _pToggleMesh.setup("toggleMesh", "") );
        _gui.loadFromFile("guiSettings.xml");
        
        // theta
        _thetasMaganer.setup();
    }
    
    void update()
    {
        // theta
        _thetasMaganer.update();
        
        // LiDAR
        CoordVec2 frame;
        _lider.extractData( frame );
    
        int frameSize = frame.size();
        if( 0 < frameSize )
        {
            float elapsedTime = ofGetElapsedTimef();
            _liderFps = getLiderFps( elapsedTime );
            
            // cout << "frameSize:" <<  frameSize << endl;
            
            
            // create mesh
            _mesh.clear();
            int coordInterval = _pCoordInterval;
            int distThread = _pDistThread;
            int imgWidth = _thetasMaganer.ThetaWidth;
            int imgHeight = _thetasMaganer.ThetaHeight;
            float widthRate = _pTexWidthRate;
            float heightRate = _pTexHeightRate;
            for( int zindex = 0; zindex < frameSize - 1 ; zindex++ )
            {
                int coordSize = frame[zindex].size();
                for( int coordId = 0; coordId < coordSize - coordInterval; coordId += coordInterval )
                {
                    Coord c1 = frame[zindex][coordId];
                    Coord c2 = frame[zindex][coordId+coordInterval];
                    Coord c3 = frame[zindex+1][coordId];
                    Coord c4 = frame[zindex+1][coordId+coordInterval];
                    
                    int dist1 = abs(c1.dist - c2.dist) + abs(c1.dist - c4.dist) + abs(c2.dist - c4.dist);
                    int dist2 = abs(c1.dist - c3.dist) + abs(c1.dist - c4.dist) + abs(c3.dist - c4.dist);
                    int aziDist = abs(c1.azimuth - c2.azimuth) ;
                    
                    // 例外処理
                    if( 50 < aziDist )
                    {
                        continue;
                    }
                    
                    float tx1Rate = c1.azimuth/360.0+widthRate;
                    tx1Rate = (1.0 < tx1Rate) ? tx1Rate - 1.0 : tx1Rate;
                    float tx2Rate = c2.azimuth/360.0+widthRate;
                    tx2Rate = (1.0 < tx2Rate) ? tx2Rate - 1.0 : tx2Rate;
                    
                    float tx1 = (1.0 - tx1Rate)*imgWidth;
                    float tx2 = (1.0 - tx2Rate)*imgWidth;
                    
                    float ty1 = (1.0 - ((zindex/(float)frameSize)*heightRate + (1.0-heightRate)*0.5 + _pTexHeightRate2))*imgHeight;
                    float ty2 = (1.0 - (((zindex+1)/(float)frameSize)*heightRate + (1.0-heightRate)*0.5 + _pTexHeightRate2))*imgHeight;
                    
                    ofVec2f t1 = ofVec2f( tx1, ty1 );
                    ofVec2f t2 = ofVec2f( tx2, ty1 );
                    ofVec2f t3 = ofVec2f( tx1, ty2 );
                    ofVec2f t4 = ofVec2f( tx2, ty2 );
                    
                    if(c1.dist != 0
                    && c2.dist != 0
                    && c4.dist != 0
                    && dist1 < distThread )
                    {
                        // vertex
                        _mesh.addVertex( c1.position );
                        _mesh.addVertex( c2.position );
                        _mesh.addVertex( c4.position );
                        
                        // texture
                        if( _pToggleMesh )
                        {
                            _mesh.addTexCoord( t1 );
                            _mesh.addTexCoord( t2 );
                            _mesh.addTexCoord( t4 );
                        }
                        // color
                        else
                        {
                            float hue = ofMap(frame[zindex][coordId].intensity, 0.0, 100.0, 255.0, 125.0, true);
                            ofColor color = ofColor::fromHsb(hue, 255, 255);
                            _mesh.addColor( color );
                            _mesh.addColor( color );
                            _mesh.addColor( color );
                        }
                    }
                    
                    if(c1.dist != 0
                    && c4.dist != 0
                    && c3.dist != 0
                    && dist2 < distThread)
                    {
                        // vertex
                        _mesh.addVertex( c1.position );
                        _mesh.addVertex( c4.position );
                        _mesh.addVertex( c3.position );
                        
                        // texture
                        if( _pToggleMesh )
                        {
                            _mesh.addTexCoord( t1 );
                            _mesh.addTexCoord( t4 );
                            _mesh.addTexCoord( t3 );
                        }
                        // color
                        else
                        {
                            float hue = ofMap(frame[zindex][coordId].intensity, 0.0, 100.0, 255.0, 125.0, true);
                            ofColor color = ofColor::fromHsb(hue, 255, 255);
                            _mesh.addColor( color );
                            _mesh.addColor( color );
                            _mesh.addColor( color );
                        }
                    }
                    
                }
            }
            
            //_mesh.setupIndicesAuto(); //ひつよう？
            _mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        }
    }
    
    
    void draw()
    {
        ofClear(0);
        ofSetColor( 255, 255, 255 );
        
        if( Const::DEBUG_MODE == MODE_DEBUG )
        {
            // draw
            _cam.begin();
            {
                // texture
                if( _pToggleMesh )
                {
                    _thetasMaganer._fbo.getTextureReference().bind();
                    {
                        _mesh.draw();
                    }
                    _thetasMaganer._fbo.getTextureReference().unbind();
                }
                else
                {
                    _mesh.drawWireframe();
                }
            }
            _cam.end();
        }
        
        
        // gui
        _gui.draw();

        
        // fps
        stringstream ss;
        ss << "framerate: " << ofToString(ofGetFrameRate(), 0) << endl;
        ss << "lidar fps: " << ofToString(_liderFps) << endl;
        //ss << "debug mode: " << Const::DEBUG_MODE << endl;
        ofDrawBitmapString(ss.str(), 10, 20);
    }
    
    
    void keyPressed(int key)
    {
        if(key == 'f')
        {
            ofToggleFullscreen();
        }
    }
    
    void exit()
    {
        _lider.exit();
    }
    
    int getLiderFps(float elapsedTime)
    {
        static int preElapsedTime = 0;
        static int fps = 0;
        static int count = 0;

        if(  preElapsedTime < int(elapsedTime)  )
        {
            fps = count;
            count = 0;
        }
        
        count++;
        preElapsedTime = int(elapsedTime);
        return fps;
    }
    
    
};
