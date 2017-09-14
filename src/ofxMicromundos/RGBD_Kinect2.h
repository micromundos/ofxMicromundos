#pragma once

#include "ofxMicromundos/RGBD.h"
#include "ofxKinectV2.h"
#include "ofxCv.h"

namespace ofxMicromundos { 

  class RGBD_Kinect2 : public RGBD
  {
    public:

      RGBD_Kinect2() {};

      ~RGBD_Kinect2()
      {
        dispose();
      };

      void init()
      {
        vector <ofxKinectV2::KinectDeviceInfo> deviceList = kinect.getDeviceList();
        if (deviceList.empty())
          ofLogError("kinect device list is empty");
        kinect.open(deviceList[0].serial);
        kinect.setRegistration(true);
        kinect.minDistance = 100; 
        kinect.maxDistance = 2000; 
      };

      void update()
      {
        kinect.update();
        _kinect_updated = kinect.isFrameNew();

        if (_kinect_updated)
        {
          depth_pix = kinect.getRawDepthPixels(); //copy
          ofxCv::flip(depth_pix, depth_pix, 1);

          depth_pix_undist = kinect.getRawDepthPixelsUndistorted(); //copy
          ofxCv::flip(depth_pix_undist, depth_pix_undist, 1);

          grey_pix = kinect.getDepthPixels(); //copy
          ofxCv::flip(grey_pix, grey_pix, 1);

          rgb_pix = kinect.getRgbPixels(); //copy
          ofxCv::flip(rgb_pix, rgb_pix, 1);
        }
      };

      void dispose()
      {
        kinect.close();
      }; 

      void render_grey_depth(float x, float y, float w, float h)
      {
        if (_kinect_updated)
          grey_tex.loadData(grey_pix);
        grey_tex.draw(x, y, w, h);
      };

      void render_rgb(float x, float y, float w, float h)
      {
        if (_kinect_updated)
          rgb_tex.loadData(rgb_pix);
        rgb_tex.draw(x, y, w, h);
      };

      ofFloatPixels& depth_pixels_undistorted()
      {
        return depth_pix_undist;
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

      // kinect2 3d units: mts
      ofVec3f point(int x, int y)
      {
        int _x = x; //depth_pix.getWidth()-1 - x; //flipped
        return kinect.getWorldCoordinateAt(_x, y);
      };

      //vector<ofVec3f>& point_cloud()
      //{
        //TODO RGBD_Kinect2: implement point cloud
        //1) calc 3d points with ofxGPGPU using pinhole projection:
        //libfreenect2::registration::getPointXYZRGB(r, c) {
          //float depth_val = undistorted_data[512*r+c]/1000.0f;
          //x = (c + 0.5 - cx) * fx * depth_val;
          //y = (r + 0.5 - cy) * fy * depth_val;
          //z = depth_val;
        //}
        //2) in ofxKinectV2
      //}; 

      bool depth_updated()
      {
        return _kinect_updated;
      };

      bool rgb_updated()
      {
        return _kinect_updated;
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

      ofxKinectV2 kinect;

      bool _kinect_updated;

      ofFloatPixels depth_pix;
      ofFloatPixels depth_pix_undist;
      ofPixels grey_pix;
      ofPixels rgb_pix;

      ofTexture rgb_tex;
      ofTexture grey_tex;
  };

};

