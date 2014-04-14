#pragma once

#include "ofMain.h"
#include "ofxVoiceSynthesizer.h"
#include "series.h"
#include "thought.h"
#include "ofxOsc.h"
#include "ofxTwitter.h"


#define WATER       0
#define EARTH       1
#define AIR         2

#define TRIP_STATE_WAITING_FOR_VIEWER   0
#define TRIP_STATE_INTRODUCTION         1
#define TRIP_STATE_PLAYING              2
#define TRIP_STATE_OUTRO                3
#define TRIP_STATE_FLASH                4


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    
        void loadDialogue();
        void loadTweets();
        void nextSeriesBasedOnThoughts();
        void startTrip();
        void introduction();
        void outro();
        void flash();
        string takePhoto();
        int randomUninterestingTopic(int interestedTopic);
        int randomThoughtIndex(int topic);
        int favoriteTopic();
    
        ofTrueTypeFont text;
        int tripState;
        float flashTime;
    
        //thought based architecture
        vector < vector<Thought> > thoughts;
        Series series;
        
        //other
    
        vector<int> brainWaveStrength;
        ofSoundPlayer introSound;
        ofImage heartImage;
        int heartbeatDirection;
        int selectedTopic;
        float heartbeatAlpha;
        float heartbeatTempo;
        float introSoundFadeOut;
        ofSoundPlayer outroSound;
        string viewerImage;
        string viewerName;
        vector < vector<string> > tweets;
        int thoughtsPerSeries;
        int highInterestDisplay;
    
        //true for all
        ofxVoiceSynthesizer synth;
        ofxOscReceiver receiver;
        ofXml XML;
        ofxTwitter twitterClient;
        bool tookPhotoWithFlash;
        bool usingRandomValues;
        bool usingTwitter;
    
        
        //webcam
        
        ofVideoGrabber 		camera;
        ofImage             viewer;
        
        //debugging features
        
        bool debug;

};
