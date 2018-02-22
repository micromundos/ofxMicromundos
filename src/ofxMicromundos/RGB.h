#pragma once

#include "ofxCv.h"

class RGB
{
  public:

    RGB() {};
    ~RGB() 
    {
      dispose();
    };

    void init(float w, float h, int device_id)
    {
      cam.setDeviceID(device_id);
      cam.setDesiredFrameRate(30);
      cam.setup(w, h);
      _updated = false;
    }; 

    bool update()
    {
      cam.update();
      _updated = cam.isFrameNew();
      return _updated;
      //pix = cam.getPixels();
      //ofxCv::flip(pix, pix, 1); //some cameras need flipping
    };

    void render(float x, float y, float w, float h)
    {
      if (_updated)
        tex.loadData(cam.getPixels());
      if (tex.isAllocated())
        tex.draw(x, y, w, h);
    };

    void dispose()
    {
      cam.close();
      //pix.clear();
      tex.clear();
    };

    ofPixels& pixels()
    {
      return cam.getPixels();
    };

    int width()
    {
      return cam.getPixels().getWidth();
    };

    int height()
    {
      return cam.getPixels().getHeight();
    };

  private:

    //ofPixels pix;
    ofVideoGrabber cam;
    ofTexture tex;
    bool _updated;

};

