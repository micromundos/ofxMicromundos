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
          depth_pix = astra.getRawDepth(); //copy
          ofxCv::flip(depth_pix, depth_pix, 1);
          grey_pix = astra.getDepthImage().getPixels(); //copy
          ofxCv::flip(grey_pix, grey_pix, 1);
        }

        grabber.update();
        _rgb_updated = grabber.isFrameNew();

        if (_rgb_updated)
        {
          rgb_pix = grabber.getPixels(); //copy
          ofxCv::flip(rgb_pix, rgb_pix, 1);
        }
      };

      void dispose()
      {
        grabber.close();
        //astra.close();
      }; 

      void render_grey_depth(float x, float y, float w, float h)
      {
        astra.enableDepthImage(true);
        //astra.drawDepth(x, y, w, h);
        if (_depth_updated)
        {
          grey_tex.loadData(grey_pix);
          //grey_tex.loadData(depth_pix);
        }
        grey_tex.draw(x, y, w, h);
      };

      void render_rgb(float x, float y, float w, float h)
      {
        if (_rgb_updated)
          rgb_tex.loadData(rgb_pix);
        rgb_tex.draw(x, y, w, h);
      };

      ofFloatPixels& depth_pixels()
      {
        return depth_pix;
      };

      ofPixels& grey_pixels()
      {
        return grey_pix;
      };

      ofPixels& rgb_pixels()
      {
        return rgb_pix;
      };

      // astra 3d units: mm
      ofVec3f point(int x, int y)
      {
        int _x = depth_pix.getWidth()-1 - x; //flipped
        return astra.getWorldCoordinateAt(_x, y);
      };

      //TODO RGBD_Astra flipped point cloud
      //vector<ofVec3f>& point_cloud()
      //{
        //return astra.getPointCloud();
      //}; 

      bool depth_updated()
      {
        return _depth_updated;
      };

      bool rgb_updated()
      {
        return _rgb_updated;
      }; 

      int depth_width()
      {
        return depth_pix.getWidth();
      };

      int depth_height()
      {
        return depth_pix.getHeight();
      };

      int rgb_width()
      {
        return rgb_pix.getWidth();
      };

      int rgb_height()
      {
        return rgb_pix.getHeight();
      };

    private:

      ofxOrbbecAstra astra;
      ofVideoGrabber grabber;

      bool _rgb_updated;
      bool _depth_updated;

      ofFloatPixels depth_pix;
      ofPixels grey_pix;
      ofPixels rgb_pix;

      ofTexture rgb_tex;
      ofTexture grey_tex;
  };

};

