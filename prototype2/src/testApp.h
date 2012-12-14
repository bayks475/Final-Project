#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

#define NUM_STRIPS 10




class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    bool bTakeSnapshot;
    //video
      ofVideoGrabber video;
      int videoW;
      int videoH;

      int t;
      int blur;
    
    //slitscan

    int maxBufferFrames;
    int stripHeight;
    vector<ofTexture> frameBuffers;
    int frameDelay[NUM_STRIPS];


    //opencv
    ofxCvColorImage colorVideo;
    ofxCvGrayscaleImage grayVideo;
    ofxCvGrayscaleImage background;
    ofxCvGrayscaleImage diffVideo;

    //blob
    ofxCvContourFinder contourVideo;
     
    float twp;
    float wpp;

//sound


void audioOut(float * output, int bufferSize, int nChannels);


ofSoundStream soundStream;

float 	pan;
int		sampleRate;
bool 	bNoise;
float 	volume;

vector <float> lAudio;
vector <float> rAudio;

//------------------- for the simple sine wave synthesis
float 	targetFrequency;
float 	phase;
float 	phaseAdder;
float 	phaseAdderTarget;

//should be an object
    vector < float > phases;
    vector < float > phaseAdders;
    vector < float > phaseAdderTargets;
    
    vector < float > curRecording;
    vector < vector < float > > recordings;
    float duration;
    float startTime;
    
   
    


};
