#include "testApp.h"

//float [tones] = {12.70, 16.71, 21.20, 23.65, 29.00, 35.00, 41.74, 45.41};

//--------------------------------------------------------------
void testApp::setup(){
    
    video.initGrabber(320, 240);
    videoW=video.getWidth();
    videoH=video.getHeight();
    
    t= 60;
    
    //allocate memory for the texture
    colorVideo.allocate(320, 240);
    grayVideo.allocate(320, 240);
    background.allocate(320, 240);
    diffVideo.allocate(320, 240);
    
    //background
    bTakeSnapshot = true;
    
    int delayIncrement = 5; // How much should the delay increase per strip?
    
    
    // Set the frame delay for each strip
    for(int i=0; i<NUM_STRIPS; i++) {
        frameDelay[i] = i * delayIncrement;  // It will be 50 delay at the top, and 0 frame delay at the bottom
    }
    
    
    // Calculate how many old frames from the camera we should keep around
    maxBufferFrames = NUM_STRIPS * delayIncrement;
    
    // How tall should each strip be?
    stripHeight = video.getHeight() / NUM_STRIPS;

    //sound
    int bufferSize		= 512;
	sampleRate 			= 44100;
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
    
    lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
    
    soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);
    
    ofSetFrameRate(60);
    
    duration = 2;
    startTime = ofGetElapsedTimef();
    
    
}

//--------------------------------------------------------------
void testApp::update(){

    video.update();
   



    if(video.isFrameNew()){
        colorVideo.setFromPixels(video.getPixels(), videoW, videoH);
        
        // If we have a new frame, add it to the end of the buffer
        ofTexture frame;
        frame.allocate(320, 240, GL_RGB);
        frame.loadData(video.getPixels(), 320, 240, GL_RGB);
        frameBuffers.push_back( frame );
        
        // If we have more than "maxBufferFrames" in the buffer, get rid of the oldest one
        if(frameBuffers.size() > maxBufferFrames) {
            frameBuffers.erase(frameBuffers.begin());
        }

        
        
        
        //background subtractions
        colorVideo.mirror(false, true);
        grayVideo=colorVideo;
        
        if(bTakeSnapshot) {
            background = grayVideo;
            bTakeSnapshot = false;
        }
        
        diffVideo.absDiff(background, grayVideo);
        
        diffVideo.threshold(t);
        diffVideo.dilate();
        diffVideo.dilate();
        
        
    //contourVideo.findContours(grayVideo, 0, videoW*videoH, 1, true); //find contours
    contourVideo.findContours(diffVideo, 0, videoW*videoH, 1, false); //find contours


    //add treshold
    grayVideo.threshold(t);
    
    //white pixels
    twp = grayVideo.countNonZeroInRegion(0, 0, videoW , videoH); // get the number of non-black pixels
    wpp = twp / (videoW*videoH); // get the percentage
        
        
        //map the percentage
        targetFrequency = ofMap(wpp, 0, 1, 50, 2000);
        phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
   
    }
    
}

//--------------------------------------------------------------
void testApp::draw(){
    // Draw all of the stips
    
    for(int i=0; i<NUM_STRIPS; i++)
    {
        if(frameDelay[i] >= frameBuffers.size()) continue; // if we don't have enough buffered frames yet, skip drawing it.
        int n = frameBuffers.size()-frameDelay[i]-1;  // Which buffered frame should we draw?
        
        int y = i * stripHeight;
        ofSetColor(255);
        frameBuffers[n].drawSubsection(0, y, frameBuffers[n].getWidth(), stripHeight, 0, y);
        
    }
    
    

    //draw it
    //video.draw(0, 0, videoW, videoH);
    
    //textures
    colorVideo.draw(videoW,0);
    grayVideo.draw(videoW*2, 0);
    
    //blob
     contourVideo.draw(videoW*3,0,videoW,videoH);
    for(int i=0; i<contourVideo.nBlobs; i++)
    {
        ofxCvBlob blob = contourVideo.blobs[i];
        vector<ofPoint> contour = blob.pts;
        for(int j=0; j<contour.size(); j++)
        {
            float x = ofMap(contour[j].x, 0, videoW, 0, videoH/2);
            float y = ofMap(contour[j].y, 0, videoW, 0, videoH/2);
           
        }
    }
    
    
    //check it
    ofDrawBitmapString("totalWhitePixels: " + ofToString(twp), 500, 500);
    ofDrawBitmapString("whitePixels Pct: " + ofToString(wpp), 500, 600);
    
    
    //ofRect(0, videoH, 100*wpp, 100*wpp);//use the pct to resize a square


//sound check
    ofSetColor(225);
	string reportString = "volume: ("+ofToString(volume, 2)+") modify with -/+ keys\npan: ("+ofToString(pan, 2)+") modify with mouse x\nsynthesis: ";
	if( !bNoise ){
		reportString += "sine wave (" + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	}else{
		reportString += "noise";
	}
	ofDrawBitmapString(reportString, 32, 579);


}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    if(key== ' ') {
        bTakeSnapshot = true;
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::audioOut(float * output, int bufferSize, int nChannels) {
    
    //pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;
    
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
    
    for (int i = 0; i < phases.size(); i++){
        while (phases[i] > TWO_PI){
            phases[i] -= TWO_PI;
        }
    }
    
    
    
    for (int i = 0; i < phaseAdders.size(); i++){
        
        float pct = (ofGetElapsedTimef() - startTime) / duration;
        
        int sample = (int)ofMap(pct, 0, 1, 0, recordings[i].size()-1, true);
        targetFrequency =    recordings[i][ sample ]; //// ofMap(wpp, 0, 1, 50, 2000);
        phaseAdderTargets[i] = (targetFrequency / (float) sampleRate) * TWO_PI;
    }

    
    
	if ( bNoise == true){
		// ---------------------- noise --------------
		for (int i = 0; i < bufferSize; i++){
			lAudio[i] = output[i*nChannels    ] = ofRandom(0, 1) * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = ofRandom(0, 1) * volume * rightScale;
		}
	} else {
		
       
        phaseAdder = 0.95f * phaseAdder + 0.07f * phaseAdderTarget;
		for (int i = 0; i < bufferSize; i++){
			phase += phaseAdder;
			float sample = sin(phase);
            
			lAudio[i] = output[i*nChannels    ] = sample * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = sample * volume * rightScale;
		}
        
        // add all the other video sin waves....
        
        for (int i = 0; i < phaseAdders.size(); i++){
            phaseAdders[i] = 0.95f * phaseAdders[i] + 0.07f * phaseAdderTargets[i];
            for (int j = 0; j < bufferSize; j++){
                phases[i] += phaseAdders[i];
                float sample = sin(phases[i]);
                
                output[j*nChannels    ] += sample * volume * leftScale;
                output[j*nChannels + 1] += sample * volume * rightScale;
            }
        }
	}
    
   /*
    if (ofGetElapsedTimef() - startTime > duration){
        startTime = ofGetElapsedTimef();
        printf("new sound recording \n");
        
        recordings.push_back(curRecording);
        
        
        phases.push_back(0);
        phaseAdders.push_back(0);
        phaseAdderTargets.push_back(0);
        
        
        curRecording.clear();
        
        
        if (recordings.size() > 6){
            recordings.erase(recordings.begin());
            phases.erase(phases.begin());
            phaseAdders.erase(phaseAdders.begin());
            phaseAdderTargets.erase(phaseAdderTargets.begin());
                         
        }
        
    } else {
        curRecording.push_back(targetFrequency);
        
    }*/
}


//--------------------------------------------------------------

void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}