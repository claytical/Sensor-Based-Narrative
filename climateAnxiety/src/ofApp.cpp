#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if(XML.load("story.xml")) {
        cout << "loaded xml story file" << endl;
    }
    else {
        cout << "couldn't load story file" << endl;
    }
    XML.setTo("//topics");

    debug = XML.getBoolValue("settings/debug");
	receiver.setup(XML.getIntValue("settings/port"));
    usingTwitter = XML.getBoolValue("settings/twitter");
    if (usingTwitter) {
        if (camera.listDevices().size() > 1) {
            camera.setDeviceID(XML.getIntValue("settings/camera"));
        }
        else {
            camera.setDeviceID(0);
        }
    }
    usingRandomValues = XML.getBoolValue("settings/random");
    thoughtsPerSeries = XML.getIntValue("settings/thoughts_per_series");
    highInterestDisplay = XML.getIntValue("settings/high_interest");
    text.loadFont("joystix.ttf", 32);
    ofBackground(0);
	ofSetVerticalSync(true);

    synth.setup(XML.getValue("introduction/voice"));
    introSound.loadSound(XML.getValue("introduction/sound"));
    introSound.setVolume(XML.getFloatValue("introduction/volume"));
    // heart by roxy80 http://roxy80.deviantart.com/art/Pixel-Heart-308259709
    heartImage.loadImage(XML.getValue("introduction/image"));
    outroSound.loadSound(XML.getValue("ending/sound"));
    outroSound.setVolume(XML.getFloatValue("ending/volume"));

    if (usingTwitter) {
        ofXml credentials;
        credentials.load("credentials.xml");
        credentials.setTo("oauth");
        twitterClient.authorize(credentials.getValue("consumer_key"), credentials.getValue("consumer_secret"));

        camera.setDesiredFrameRate(60);
        camera.initGrabber(320, 240);
    }
}

//--------------------------------------------------------------
void ofApp::introduction() {
    loadDialogue();
    introSoundFadeOut = 1;
    heartbeatDirection = 1;
    heartbeatAlpha = 10;
    heartbeatTempo = 2;
    tookPhotoWithFlash = false;
    introSound.play();
    XML.setTo("//topics");
    synth.speak(XML.getValue("introduction/text"));
    tripState = TRIP_STATE_INTRODUCTION;

}


void ofApp::flash() {
    ofSetColor(255,255,255, ofMap(ofGetElapsedTimef(), flashTime - .5, flashTime, 255, 0));
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    if (ofGetElapsedTimef() > flashTime - .4) {
        viewerImage = takePhoto();
        tookPhotoWithFlash = true;
    }
    
    if (ofGetElapsedTimef() > flashTime) {
        outro();
    }
    
}

void ofApp::outro() {
    
    if (usingTwitter) {
         loadTweets();
         int favTopic = favoriteTopic();
         int randomTweet = int(tweets[favTopic].size()-1);
         twitterClient.postStatus(tweets[favTopic][randomTweet], viewerImage);
    }
    
    if (viewerName == "this human") {
        viewerName = "Human";
    }

    XML.setTo("//topics");
    XML.setToChild(favoriteTopic());
    string outroText = XML.getValue("outro");
    synth.speak(outroText);
    outroSound.play();
    
    tripState = TRIP_STATE_OUTRO;
    
}

void ofApp::startTrip() {
    viewerName = "this human";
    brainWaveStrength.clear();
    for (int i = 0; i < thoughts.size(); i++) {
        brainWaveStrength.push_back(0);
    }
    series.reset();

    for (int i = 0; i < thoughts.size(); i++) {
        series.add(thoughts[i][0]);
    }

    for (int i = 0; i < thoughts.size(); i++) {
        thoughts[i].erase(thoughts[i].begin());
    }
    //start main sequence
    tripState = TRIP_STATE_PLAYING;
    series.start();
}

void ofApp::loadTweets() {
    
    tweets.clear();
    XML.setTo("//topics"); //at the root of topics
    int numTopics = XML.getNumChildren("topic"); // how many do you have?
    cout << "I have " << numTopics << " topics " << endl;
    XML.setTo("topic");
    for (int i = 0; i < numTopics; i++) {
        
        vector<string> tweet;
        
        int numTweets = XML.getNumChildren("tweet");
        
        for (int j = 0; j < numTweets; j++) {
            tweet.push_back(XML.getValue("tweet["+ofToString(j)+"]"));
        }

        tweets.push_back(tweet);
        XML.setToSibling(); //advance to next topic
    }


}

void ofApp::loadDialogue() {

    thoughts.clear();
    XML.setTo("//topics"); //at the root of topics
    int numTopics = XML.getNumChildren("topic"); // how many do you have?
    cout << "I have " << numTopics << " topics " << endl;
    XML.setTo("topic");
    for (int i = 0; i < numTopics; i++) {
        
        vector<Thought> topic;

        int numVideos = XML.getNumChildren("video");
        
        for (int j = 0; j < numVideos; j++) {
            Thought thought;
            string filename = XML.getValue("video["+ofToString(j)+"]");
            cout << "loading " << filename << endl;
            thought.create(filename, i, j);
            topic.push_back(thought);
        }

        thoughts.push_back(topic);
        XML.setToSibling(); //advance to next topic
    }


}




//--------------------------------------------------------------
void ofApp::update(){
    int brainWave = 0;
    bool pulse = false;
    camera.update();
    
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        
        if (m.getAddress() == "/attention") {
            brainWave = m.getArgAsInt32(0);
            
        }
        
        if (m.getAddress() == "/pulse") {
            //got pulse, let's go!
            pulse = true;
            
        }
        
        if (m.getAddress() == "/name") {
            viewerName = m.getArgAsString(0);
        }
        
        
    }
    
    switch (tripState) {
        case TRIP_STATE_WAITING_FOR_VIEWER:
            if (pulse) {
                introduction();
            }
            
            break;
        case TRIP_STATE_INTRODUCTION:
            if (!synth.isSpeaking()) {
                startTrip();
            }
            break;
        case TRIP_STATE_PLAYING:
            
            introSoundFadeOut *= .995;
            introSound.setVolume(introSoundFadeOut);
            if (introSoundFadeOut <= .01) {
                introSound.stop();
            }
            if (series.finished) {
                cout << "series finished in update" << endl;
                nextSeriesBasedOnThoughts();
            }
            if (usingRandomValues) {
                series.gaugeInterest(ofRandom(2));
                
            }
            else {
                series.gaugeInterest(brainWave);
            }
            
            break;
        case TRIP_STATE_OUTRO:
            while(receiver.hasWaitingMessages()){
                // get the next message, clear the queue
                ofxOscMessage m;
                receiver.getNextMessage(&m);
            }
            
            if (!synth.isSpeaking()) {
                tripState = TRIP_STATE_WAITING_FOR_VIEWER;
                outroSound.stop();
            }
            break;
    }
    

}

//--------------------------------------------------------------
void ofApp::draw(){
    switch (tripState) {
        case TRIP_STATE_WAITING_FOR_VIEWER:
            ofSetColor(255, 255, 255);
            if (debug) {
                camera.draw(ofGetWidth()/2 - 160,20);
            }
            text.drawString("please put your", ofGetWidth()/2 - (text.stringWidth("please put your")/2), ofGetHeight()/2);
            text.drawString("hands on the globes", ofGetWidth()/2 - (text.stringWidth("hands on the globes")/2), ofGetHeight()/2 + 40);
            
            break;
        case TRIP_STATE_INTRODUCTION:
            
            heartbeatAlpha = heartbeatAlpha + (heartbeatTempo * heartbeatDirection);
            if (heartbeatAlpha > 100) {
                heartbeatDirection = -1;
            }
            if(heartbeatAlpha < 10) {
                heartbeatDirection = 1;
                heartbeatTempo *= .95;
            }
            
            ofSetColor(255, 255, 255, heartbeatAlpha);
            heartImage.draw(ofGetWidth()/2 - heartImage.width/2, ofGetHeight()/2 - heartImage.height/2);
            break;
        case TRIP_STATE_PLAYING:
            ofSetColor(255, 255, 255);
            series.display();
            break;
        case TRIP_STATE_FLASH:
            if (usingTwitter) {
                flash();
            }
            break;
        case TRIP_STATE_OUTRO:
            ofSetColor(255, 255, 255);
            break;
    }
    

}



int ofApp::favoriteTopic() {
    float highestInterestLevel = -999999;
    int highestInterestTopic = -1;
    for (int i = 0; i < brainWaveStrength.size(); i++) {
        if (brainWaveStrength[i] > highestInterestLevel) {
            highestInterestTopic = i;
            highestInterestLevel = brainWaveStrength[i];
        }
    }
    selectedTopic = highestInterestTopic;
    return highestInterestTopic;
    
}
int ofApp::randomThoughtIndex(int type) {
    int selectedIndex;
    selectedIndex = ofRandom(thoughts[type].size());
    return selectedIndex;
}

int ofApp::randomUninterestingTopic(int interestedTopic) {
    int selectedTopic;
    //CONSTANT 3 right now is WATER, EARTH, AIR
    selectedTopic = ofRandom(thoughts.size());
    if (selectedTopic == interestedTopic) {
        return randomUninterestingTopic(interestedTopic);
    }
    else {
        return selectedTopic;
    }
}



void ofApp::nextSeriesBasedOnThoughts() {
    float highestInterestLevel = -999999;
    int highestInterestTopic = -1;
    int highestInterestIndex = -1;
    for (int i = 0; i < series.thoughts.size(); i++) {
        if (series.thoughts[i].interest > highestInterestLevel) {
            highestInterestTopic = series.thoughts[i].topic;
            highestInterestLevel = series.thoughts[i].interest;
            highestInterestIndex = i;
        }
    }
    brainWaveStrength[highestInterestTopic]++;
    if (thoughts[highestInterestTopic].size() <= 0) {
        tripState = TRIP_STATE_FLASH;
        return;
    }
    
    series.reset();
    Thought thoughtCopy;
    int index;
    int whatsLeft;
    if (thoughts[highestInterestTopic].size() < highInterestDisplay) {
        whatsLeft = thoughtsPerSeries - thoughts[highestInterestTopic].size();
        //not enough to display, run it out with whatever is left
        for (int i = 0; i < thoughts[highestInterestTopic].size(); i++) {
            series.add(thoughts[highestInterestTopic][i]);
        }
        thoughts[highestInterestTopic].clear();
    }
    else {
        whatsLeft = thoughtsPerSeries - highInterestDisplay;
        for (int i = 0; i < highInterestDisplay; i++) {
            index = randomThoughtIndex(highestInterestTopic);
            //add thought to series
            thoughtCopy = thoughts[highestInterestTopic][index];
            series.add(thoughtCopy);
            
            //delete thought from bucket
            thoughts[highestInterestTopic].erase(thoughts[highestInterestTopic].begin() + index);
        }
        
    }
    
    //choose the second or a random?
    for (int i = 0; i < whatsLeft; i++) {
        int randomTopic = randomUninterestingTopic(highestInterestTopic);
        index = randomThoughtIndex(randomTopic);
        //need to check to see if it even exists
        thoughtCopy = thoughts[randomTopic][index];
        series.add(thoughts[randomTopic][index]);
        //erase that one as well
        thoughts[randomTopic].erase(thoughts[randomTopic].begin() + index);
    }
    series.start();
}


string ofApp::takePhoto() {
    string tmpName = "viewer" + ofToString(int(ofRandom(99999999))) + ".png";
    
    ofSaveImage(camera.getPixelsRef(), tmpName);
    ofImage tmpImage;
    tmpImage.loadImage(tmpName);
    tmpImage.rotate90(3);
    tmpImage.saveImage("90"+tmpName);
    return "90" + tmpName;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'd') {
        debug = !debug;
    }
    switch (tripState) {
        case TRIP_STATE_WAITING_FOR_VIEWER:
            if (key == ' ') {
                introduction();
            }
            break;
        case TRIP_STATE_INTRODUCTION:
            break;
        case TRIP_STATE_PLAYING:
            break;
        case TRIP_STATE_OUTRO:
            break;
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
