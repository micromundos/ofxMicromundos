#pragma once

#include "ofMain.h"

namespace ofxMicromundos { 

  class RGBD 
  {
    public:

      RGBD() {};
      ~RGBD() {};

      virtual void init() = 0;
      virtual void update() = 0;
      virtual void dispose() = 0;

      virtual void render_grey_depth(float x = 0, float y = 0, float w = 640, float h = 480) = 0;
      virtual void render_rgb(float x = 0, float y = 0, float w = 640, float h = 480) = 0;

      virtual ofFloatPixels& get_depth_pixels() = 0;
      virtual ofPixels& get_grey_pixels() = 0;
      virtual ofPixels& get_rgb_pixels() = 0;
      virtual vector<ofVec3f>& get_point_cloud() = 0;
      virtual ofVec3f get_point(int x, int y) = 0;

      virtual bool rgb_updated() = 0;
      virtual bool depth_updated() = 0;
  };

};

