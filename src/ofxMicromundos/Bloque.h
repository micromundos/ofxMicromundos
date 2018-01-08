#pragma once

#include "ofMain.h"

struct Bloque
{
    int id; 
    ofVec2f loc; //normalized [0,1]
    ofVec2f dir; //normalized len vec
    float angle; //radians
    ofVec2f loc_i; //interpolated loc
    ofVec2f dir_i; //interpolated dir
    float angle_i; //interpolated angle
};

