//
// theta s の魚眼（fisheye）をパノラマに変換してfboにしてくれるクラス
//


#pragma once

class ThetasManager
{
public:
    
    // const static
    const static int ThetaWidth = 1280;
    const static int ThetaHeight = 720;
//    const static int ThetaWidth = 640;
//    const static int ThetaHeight = 360;

    
    // vars
    ofVideoGrabber _video;
    ofShader _shader;
    ofFbo _fbo;
    
    
    void setup()
    {
        // video settings
        vector<ofVideoDevice> devices = _video.listDevices();
        for(int i = 0; i < devices.size(); i++)
        {
            cout << devices[i].id << ": " << devices[i].deviceName;
            if( devices[i].bAvailable )
            {
                cout << endl;
            }
            else
            {
                cout << " - unavailable " << endl;
            }
        }
        _video.setDeviceID(1);
        _video.initGrabber(ThetaWidth, ThetaHeight);
        
        
        // fbo
        _fbo.allocate(ThetaWidth, ThetaHeight, GL_RGBA, 4);
        
        
        // shader
        _shader.load("shader/antiFishEye.vert","shader/antiFishEye.frag");
    }
    
    void update()
    {
        // video
        _video.update();
        
        // shader
        _fbo.begin();
        _shader.begin();
        {
            _shader.setUniformTexture("_tex0", _video.getTextureReference(), 0);
            _shader.setUniform1f("_width", ThetaWidth*0.5  );
            _shader.setUniform1f("_height", ThetaWidth*0.5 );
            _video.draw(0, 0);
        }
        _shader.end();
        _fbo.end();
    }
    
    void draw()
    {
        _fbo.draw(0,0);
    }
    

};