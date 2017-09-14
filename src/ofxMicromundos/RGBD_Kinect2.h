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
        _inited = false; 
      };

      void update()
      {
        kinect.update();
        _kinect_updated = kinect.isFrameNew();
        if (!_inited)
          _inited = _kinect_updated; 

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
        delete[] pcd;
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

      //TODO optimize point cloud data w/ ofxGPGPU
      float* point_cloud_data()
      {
        //vector<ofVec3f>& pcl = point_cloud();
        //float* pcd = &(pcl.data())[0].x;

        int dw = depth_width();
        int dh = depth_height();
        if (pcd == nullptr)
          pcd = new float[dw*dh*3];

        int i = 0;
        for (int y = 0; y < dh; y++)
        for (int x = dw-1; x >= 0; x--)
        {
          const ofVec3f& p = point(x, y);
          pcd[i++] = p.x;
          pcd[i++] = p.y;
          pcd[i++] = p.z;
        }

        return pcd;
      };

      //TODO impl point cloud with ofxGPGPU using libfreenect2 pinhole projection:
      /*
       * libfreenect2::registration::getPointXYZRGB(r, c) {
       *   float depth_val = undistorted_data[512*r+c]/1000.0f;
       *   x = (c + 0.5 - cx) * fx * depth_val;
       *   y = (r + 0.5 - cy) * fy * depth_val;
       *   z = depth_val;
       * }
       */
      //vector<ofVec3f>& point_cloud() {}; 

      bool inited()
      {
        return _inited;
      };

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
      bool _inited;
      float* pcd;

      ofFloatPixels depth_pix;
      ofFloatPixels depth_pix_undist;
      ofPixels grey_pix;
      ofPixels rgb_pix;

      ofTexture rgb_tex;
      ofTexture grey_tex;
  };

};

