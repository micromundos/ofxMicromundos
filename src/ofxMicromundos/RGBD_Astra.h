#pragma once

#include "ofxMicromundos/RGBD.h"
#include "ofxOrbbecAstra.h"
#include "ofxCv.h"

namespace ofxMicromundos { 

  class RGBD_Astra : public RGBD
  {
    public:

      RGBD_Astra() {};

      ~RGBD_Astra()
      {
        dispose();
      };

      void init()
      {
        astra.setup();
        astra.enableRegistration(true);
        astra.enableDepthImage(false);
        astra.initDepthStream();
        astra.initPointStream();

        grabber.setDeviceID(0);
        grabber.setDesiredFrameRate(60);
        grabber.initGrabber(1920, 1080);
      };

      void update()
      {
        astra.update();
        _depth_updated = astra.isFrameNew();

        if (_depth_updated)
        {
          ofxCv::flip(astra.getRawDepth(), depth_pixels, 1);
          ofxCv::flip(astra.getDepthImage().getPixels(), grey_pixels, 1);
        }

        grabber.update();
        _rgb_updated = grabber.isFrameNew();

        if (_rgb_updated)
        {
          ofxCv::flip(grabber.getPixels(), rgb_pixels, 1);
        }
      };

      void dispose()
      {
        grabber.close();
        //astra.close();
      }; 

      ofFloatPixels& get_depth_pixels()
      {
        return depth_pixels;
      };

      ofPixels& get_grey_pixels()
      {
        return grey_pixels;
      };

      ofPixels& get_rgb_pixels()
      {
        return rgb_pixels;
      };

      ofVec3f get_point(int x, int y)
      {
        astra.getWorldCoordinateAt(x,y);
      };

      vector<ofVec3f>& get_point_cloud()
      {
        return point_cloud;
      };

      void render_grey_depth(float x, float y, float w, float h)
      {
        astra.enableDepthImage(true);
        //astra.drawDepth(x, y, w, h);
        if (_depth_updated)
        {
          grey_tex.loadData(grey_pixels);
          //grey_tex.loadData(depth_pixels);
        }
        grey_tex.draw(x, y, w, h);
      };

      void render_rgb(float x, float y, float w, float h)
      {
        if (_rgb_updated)
          rgb_tex.loadData(rgb_pixels);
        rgb_tex.draw(x, y, w, h);
      };

      bool rgb_updated()
      {
        return _rgb_updated;
      };

      bool depth_updated()
      {
        return _depth_updated;
      };

    private:

      ofxOrbbecAstra astra;
      ofVideoGrabber grabber;

      bool _rgb_updated;
      bool _depth_updated;

      ofFloatPixels depth_pixels;
      ofPixels grey_pixels;
      ofPixels rgb_pixels;
      vector<ofVec3f> point_cloud;

      ofTexture rgb_tex;
      ofTexture grey_tex;
  };

};

