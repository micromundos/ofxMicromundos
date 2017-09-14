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

      virtual ofFloatPixels& depth_pixels() = 0;
      virtual ofPixels& grey_pixels() = 0;
      virtual ofPixels& rgb_pixels() = 0; 

      virtual ofVec3f point(int x, int y) = 0;
      virtual float* point_cloud_data() = 0;
      //virtual vector<ofVec3f>& point_cloud() = 0;

      virtual void render_grey_depth(float x = 0, float y = 0, float w = 640, float h = 480) = 0;
      virtual void render_rgb(float x = 0, float y = 0, float w = 640, float h = 480) = 0; 

      virtual void enable_grey_depth(bool enabled) = 0;

      virtual bool inited() = 0;

      virtual bool depth_updated() = 0;
      virtual bool rgb_updated() = 0; 

      virtual int depth_width() = 0;
      virtual int depth_height() = 0; 

      virtual int rgb_width() = 0;
      virtual int rgb_height() = 0;
  };

};

