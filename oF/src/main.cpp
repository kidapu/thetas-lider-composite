#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main()
{
    
    if( Const::DEBUG_MODE == MODE_DEBUG)
    {
        ofSetupOpenGL(1024,768,OF_WINDOW);
    }
    else if( Const::DEBUG_MODE == MODE_LIVE )
    {
        ofSetupOpenGL(200, 200, OF_WINDOW);
    }

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}