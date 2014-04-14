//
//  thought.cpp
//  ClimateTrip
//
//  Created by Clay Ewing on 3/17/14.
//
//

#include "thought.h"
void Thought::create(string filename, int t, int it) {
    video.loadMovie(filename);
    topic = t;
    iter = it;
    interest = 0;
    video.setVolume(1);
    video.setLoopState(OF_LOOP_NONE);
}

void Thought::start() {
    video.play();
}

bool Thought::display() {
    video.update();
    video.draw(0, 0);
    return video.isPlaying();
}
void Thought::gaugeInterest(float num) {
    interest += num;
}


