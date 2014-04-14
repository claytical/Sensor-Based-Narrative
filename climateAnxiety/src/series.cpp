//
//  series.cpp
//  ClimateTrip
//
//  Created by Clay Ewing on 3/17/14.
//
//

#include "series.h"

void Series::add(Thought thought) {
    thoughts.push_back(thought);
}
void Series::gaugeInterest(float num) {
    //series interest
    interest += num;
    //current thought interest
    thoughts[currentThoughtIndex].gaugeInterest(num);
}

void Series::display() {
    if (currentThoughtIndex < thoughts.size()) {
        if(thoughts[currentThoughtIndex].display()) {
         //still playing
        }
        else {
            //next thought
            currentThoughtIndex++;
            if (currentThoughtIndex >= thoughts.size()) {
                //next series
                finished = true;
            }
            else {
                thoughts[currentThoughtIndex].start();
            }
        }
    }
}

void Series::start() {
    started = true;
    currentThoughtIndex = 0;
    thoughts[currentThoughtIndex].start();
    finished = false;
}

bool Series::currentThoughtFinished() {
    if (!thoughts[currentThoughtIndex].video.isPlaying()) {
        return true;
    }
    else {
        return false;
    }
}

void Series::reset() {
    thoughts.clear();
    currentThoughtIndex = 0;
    started = false;
    finished = false;
}


bool Series::nextThought() {
    if (currentThoughtIndex < thoughts.size() - 1) {
        currentThoughtIndex++;
        thoughts[currentThoughtIndex].video.play();
        return true;
    }
    else {
        
        return false;
    }
}
