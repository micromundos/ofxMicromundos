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

        kinect.setMapDepthPixels(false);
        _grey_depth_enabled = false;
      };

      void update()
      {
        kinect.update();
        _kinect_updated = kinect.isFrameNew();  

        if (_kinect_updated)
        {
          depth_pix = kinect.getRawDepthPixels(); //getRawDepthPixelsUndistorted();
          ofxCv::flip(depth_pix, depth_pix, 1);

          rgb_pix = kinect.getRgbPixels();
          ofxCv::flip(rgb_pix, rgb_pix, 1);

          if (_grey_depth_enabled)
          {
            grey_pix = kinect.getDepthPixels();
            ofxCv::flip(grey_pix, grey_pix, 1);
          } 
        }

        if (!_inited)
          _inited = _kinect_updated;

        //if (_inited && rgb_to_depth_coord_map == nullptr)
          //init_rgb_to_depth_coord_map();
        //if (_kinect_updated)
          //update_rgb_to_depth_coord_map();
      };

      void dispose()
      {
        delete[] pcd;
        delete[] rgb_to_depth_coord_map;
        kinect.close();
      }; 

      void render_grey_depth(float x, float y, float w, float h)
      {
        if (!_grey_depth_enabled)
          return;
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

      void render_rgb_on_depth(float x, float y, float w, float h)
      {
        if (_kinect_updated)
        {
          ofFloatPixels rgb_on_depth_pix = kinect.getRgbOnDepthPixels(); 

          int dw = depth_width();
          int dh = depth_height();
          int dlen = dw*dh;

          if (!rgb_on_depth_pix3.isAllocated())
            rgb_on_depth_pix3.allocate(dw, dh, 3);

          //int j = 0;
          //for (int i = 0; i < dlen; i++)
          for (int dy = 0; dy < dh; dy++)
          for (int dx = 0; dx < dw; dx++)
          {
            rgb_on_depth_pix3.setColor(dx, dy, kinect.getColorAt(dx, dy));
            //int di = to_idx(dx, dy, dw);
            //float rgb = rgb_on_depth_pix[di];
            //const uint8_t *p = reinterpret_cast<uint8_t*>(&rgb);
            //uint8_t b = p[0];
            //uint8_t g = p[1];
            //uint8_t r = p[2];
            //rgb_on_depth_pix3.setColor(dx, dy, ofColor(r, g, b));
            //rgb_on_depth_pix3[j++] = r;
            //rgb_on_depth_pix3[j++] = g;
            //rgb_on_depth_pix3[j++] = b;
          }

          ofxCv::flip(rgb_on_depth_pix3, rgb_on_depth_pix3, 1);

          rgb_on_depth_tex.loadData(rgb_on_depth_pix3);
        }
        rgb_on_depth_tex.draw(x, y, w, h);
      };

      void render_depth_on_rgb(float x, float y, float w, float h)
      {
        //kinect.getDepthOnRgbPixels:
        //if (_kinect_updated)
        //{
          //ofPixels depth_on_rgb_pix = kinect.getDepthOnRgbPixels();
          //ofxCv::flip(depth_on_rgb_pix, depth_on_rgb_pix, 1);
          //depth_on_rgb_tex.loadData(depth_on_rgb_pix);
        //}
        //depth_on_rgb_tex.draw(x, y, w, h);

        //if (rgb_to_depth_coord_map == nullptr)
          //return;
        //if (_kinect_updated)
        //{
          //int cw = rgb_width();
          //int ch = rgb_height(); 
          //int clen = cw*ch; 
          //for (int i = 0; i < clen; i++)
          //{
            //int di = rgb_to_depth_coord_map[i];
            //depth_on_rgb_pix[i] = di > -1 ? grey_pix[di] : 0;
          //}
          //depth_on_rgb_tex.loadData(depth_on_rgb_pix);
        //}
        //depth_on_rgb_tex.draw(x, y, w, h);
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
        int _x = x; //depth_width()-1 - x; //flip
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

      void enable_grey_depth(bool enabled) 
      {
        kinect.setMapDepthPixels(enabled);
        _grey_depth_enabled = enabled;
      };

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
        return depth_pixels().getWidth();
      };

      int depth_height()
      {
        return depth_pixels().getHeight();
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
      bool _grey_depth_enabled;

      float* pcd;

      ofTexture rgb_on_depth_tex;
      ofPixels rgb_on_depth_pix3;

      int* rgb_to_depth_coord_map;
      ofPixels depth_on_rgb_pix;
      ofTexture depth_on_rgb_tex;

      ofFloatPixels depth_pix;
      ofPixels grey_pix;
      ofPixels rgb_pix;

      ofTexture rgb_tex;
      ofTexture grey_tex;

      void init_rgb_to_depth_coord_map()
      {
        if (!_inited)
        {
          ofLogError() << "RGBD_Kinect2 init_rgb_to_depth_coord_map: kinect not initialized";
          return;
        }

        int cw = rgb_width();
        int ch = rgb_height(); 
        int clen = cw*ch; 

        rgb_to_depth_coord_map = new int[clen]; 
        depth_on_rgb_pix.allocate(cw, ch, 1);
      };

      void update_rgb_to_depth_coord_map()
      {

        int cw = rgb_width();
        int ch = rgb_height();
        int clen = cw*ch;

        memset(rgb_to_depth_coord_map, -1, sizeof(int)*clen);

        int dw = depth_width();
        int dh = depth_height();
        int dlen = dw*dh;

        for (int dy = 0; dy < dh; dy++)
        for (int dx = 0; dx < dw; dx++)
        {
          int depth_idx = to_idx(dx, dy, dw);
          ofVec2f c = kinect.getColorCoordinateAt(dx,dy);
          //c.x *= cw;
          //c.y *= ch;
          int rgb_idx = to_idx(int(c.x), int(c.y), cw);
          ofLog() << dx << ", " << dy << " rgb: " << c << " rgb_idx: " << rgb_idx;
          if (!valid(c.x, c.y))
            continue;
          //rgb_to_depth_coord_map[rgb_idx] = depth_idx;
        }

        //int* depth_to_rgb_map = kinect.getDepthToColorMap();
        //for (int i = 0; i < dlen; i++)
        //{
          //int rgb_idx = depth_to_rgb_map[i];
          //rgb_to_depth_coord_map[rgb_idx] = i;
        //}

        //ofLog() << "init rgb_to_depth_coord_map len " << clen;
        //for (int i = 0; i < clen; i++)
        //{
          //int di = rgb_to_depth_coord_map[i];
          //ofVec2f c = to_xy(i, cw, ch);
          //ofLog() << "\t" << "rgb i: " << i << " coord: " << c << " = " <<  " depth i: " << di << " coord: " << to_xy(di, dw, dh);
        //}
      }; 

      bool valid(int x, int y)
      {
        return !isnan(x) && !isnan(y);
      };

      ofVec2f to_xy(int i, int w, int h)
      {
        float x = ofClamp(i % w, 0, w-1);
        float y = ofClamp(float(i-x)/w, 0, h-1);
        return ofVec2f(x, y);
      };

      int to_idx(int x, int y, int w)
      {
        return x + y * w;
      };
  };

};

