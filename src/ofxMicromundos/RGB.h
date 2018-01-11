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

    //TODO RGB init camera from settings file
    void init()
    {
      vid.setDeviceID(1);
      vid.setDesiredFrameRate(30);
      vid.initGrabber(1920, 1080);
    }; 

    bool update()
    {
      vid.update();
      _updated = vid.isFrameNew();
      return _updated;
      //pix = vid.getPixels();
      //ofxCv::flip(pix, pix, 1); //some cameras need flipping
    };

    void render(float x, float y, float w, float h)
    {
      if (_updated)
        tex.loadData(vid.getPixels());
      if (tex.isAllocated())
        tex.draw(x, y, w, h);
    };

    void dispose()
    {
      vid.close();
      //pix.clear();
      tex.clear();
    };

    bool updated()
    {
      return _updated;
    };

    ofPixels& pixels()
    {
      return vid.getPixels();
    };

    int width()
    {
      return vid.getPixels().getWidth();
    };

    int height()
    {
      return vid.getPixels().getHeight();
    };

  private:

    //ofPixels pix;
    ofVideoGrabber vid;
    ofTexture tex;
    bool _updated;

};

