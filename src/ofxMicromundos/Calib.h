#pragma once

#include "ofxCv.h"
#include "ofxChilitags.h"
#include "ofxMicromundos/utils.h"
//#include "ofxGPGPU.h"

using namespace ofxCv;

class Calib
{
  public:

    Calib() {};
    ~Calib() 
    {
      dispose();
    };

    bool init(
        float proj_w, float proj_h, 
        string H_cam_proj_file, 
        string calib_cam_file, 
        int calib_tag_id)
    {
      this->H_cam_proj_file = H_cam_proj_file;
      this->calib_cam_file = calib_cam_file;
      this->calib_tag_id = calib_tag_id; 

      UP = ofVec2f(0,1);
      H_ready = false;
      H_maps_ready = false;

      //init_H_gpu();

      calib_tags.resize(4);
      for (int i = 0; i < calib_tags.size(); i++)
        calib_tags[i].load("calib/"+ofToString(i+1)+".png");

      vector<ofVec2f> _proj_pts = ofxMicromundos::calib_points(proj_w, proj_h);
      for (const auto& pt : _proj_pts)
        proj_pts.push_back(cv::Point2f(pt.x, pt.y)); 

      proj_coords.push_back(ofVec2f( -1.,-1. ));
      proj_coords.push_back(ofVec2f(  1.,-1. ));
      proj_coords.push_back(ofVec2f(  1., 1. ));
      proj_coords.push_back(ofVec2f( -1., 1. ));

      load_H();
      load_cam_calib();
    };

    bool enabled(vector<ChiliTag>& tags)
    {
      for (int i = 0; i < tags.size(); i++)
        if (is_calib_tag(tags[i]))
          return true;
      return false;
    };

    void calibrate(vector<ChiliTag>& _tags, float w, float h)
    {
      vector<ChiliTag> tags = filter_calib_tag(_tags);

      if (tags.size() != proj_pts.size())
        return;

      ofVec2f scale(w,h);

      ofVec2f ctr = tags_ctr(tags);
      tags_pts.clear(); 
      for (int i = 0; i < tags.size(); i++)
      {
        ofVec2f coord = proj_coords[i];
        ofVec2f tag_n = tag_from_proj_coord(coord, ctr, tags);
        tags_pts.push_back(toCv(tag_n * scale));
      }

      H_cv = cv::findHomography(
          cv::Mat(tags_pts), 
          cv::Mat(proj_pts));
      H_ready = true; 
      H_maps_ready = false;

      save_H();
    }; 

    void render()
    { 
      //render_calib_tags();
      render_calib_pts();
    }; 

    void transform_pix(ofPixels &src, ofPixels &dst)
    {
      if (!H_ready)
        return; 

      cv::Mat src_mat = toCv(src);
      cv::Mat dst_mat = toCv(dst);

      if (!H_maps_ready)
      {
        perspective_to_maps(H_cv, src_mat.size(), H_map_x, H_map_y);
        H_maps_ready = true;
      }

      cv::remap(src_mat, dst_mat, H_map_x, H_map_y, CV_INTER_LINEAR);
      //cv::warpPerspective(src_mat, dst_mat, H_cv, src_mat.size(), cv::INTER_LINEAR);

      //toOf(dst_mat, dst); //raspi: Segmentation fault
    };

    void transform_tags(vector<ChiliTag>& src_tags, vector<ChiliTag>& dst_tags, float w, float h)
    {
      if (!H_ready)
        return;

      ofVec2f scale(w,h); 

      dst_tags.clear();
      for (int i = 0; i < src_tags.size(); i++)
        dst_tags.push_back(transform_tag(src_tags[i], scale)); 
    };

    void undistort(ofPixels& pix)
    {
      if (!cam_calib.isReady())
        return;
      cv::Mat mat = toCv(pix);
      cam_calib.undistort(mat);
      toOf(mat, pix);
    }; 

    void dispose()
    {
      //dispose_H_gpu();
    }; 

  private:

    int calib_tag_id;
    string H_cam_proj_file;
    string calib_cam_file;

    cv::Mat H_cv;
    bool H_ready; 

    cv::Mat H_map_x, H_map_y;
    bool H_maps_ready;

    ofxCv::Calibration cam_calib;

    ofVec2f UP;
    vector<ofImage> calib_tags;

    vector<cv::Point2f> tags_pts;
    vector<cv::Point2f> proj_pts;
    vector<ofVec2f> proj_coords;

    vector<ChiliTag> filter_calib_tag(vector<ChiliTag> &_tags)
    {
      vector<ChiliTag> tags;
      for (int i = 0; i < _tags.size(); i++)
        if (!is_calib_tag(_tags[i]))
          tags.push_back(_tags[i]);
      return tags;
    };

    bool is_calib_tag(ChiliTag &tag)
    {
      return tag.id == calib_tag_id;
    };

    void transform_pts(ofVec2f &src, ofVec2f &dst)
    {
      vector<ofVec2f> src_pts;
      src_pts.push_back(src);
      vector<ofVec2f> dst_pts;
      dst_pts.push_back(dst);
      transform_pts(src_pts, dst_pts);
      dst.set(dst_pts[0]);
    }; 

    void transform_pts(vector<ofVec2f> &src, vector<ofVec2f> &dst)
    {
      vector<cv::Point2f> src_pts = toCv(src);
      vector<cv::Point2f> dst_pts(src.size());
      cv::perspectiveTransform(src_pts, dst_pts, H_cv);
      dst.resize(src.size());
      for(int i = 0; i < dst_pts.size(); i++) 
      {
        dst[i].x = dst_pts[i].x;
        dst[i].y = dst_pts[i].y;
      }
    };

    void transform_pts(vector<cv::Point2f> &src, vector<cv::Point2f> &dst)
    {
      cv::perspectiveTransform(src, dst, H_cv);
    }; 

    void transform_pts(ofVec2f &point)
    {
      transform_pts(point, point);
    };

    void transform_pts(vector<ofVec2f> &points)
    {
      transform_pts(points, points);
    };

    /*
     * http://www.smallbulb.net/2013/351-opencv-convert-projection-matrix-to-maps
     *
     * alternative:
     * http://romsteady.blogspot.com.ar/2015/07/calculate-opencv-warpperspective-map.html
     */
    void perspective_to_maps(const cv::Mat &perspective_mat, const cv::Size &img_size, cv::Mat &map1, cv::Mat &map2)
    {

      // invert the matrix because the transformation maps must be
      // bird's view -> original
      cv::Mat inv_perspective(perspective_mat.inv());
      inv_perspective.convertTo(inv_perspective, CV_32FC1);

      // create XY 2D array
      // (((0, 0), (1, 0), (2, 0), ...),
      //  ((0, 1), (1, 1), (2, 1), ...),
      // ...)
      cv::Mat xy(img_size, CV_32FC2);
      float *pxy = (float*)xy.data;
      for (int y = 0; y < img_size.height; y++)
        for (int x = 0; x < img_size.width; x++)
        {
          *pxy++ = x;
          *pxy++ = y;
        }

      // perspective transformation of the points
      cv::Mat xy_transformed;
      cv::perspectiveTransform(xy, xy_transformed, inv_perspective);

      // split x/y to extra maps
      assert(xy_transformed.channels() == 2);
      cv::Mat maps[2]; // map_x, map_y
      cv::split(xy_transformed, maps);

      // remap() with integer maps is faster
      cv::convertMaps(maps[0], maps[1], map1, map2, CV_16SC2);
    }

    ChiliTag transform_tag(ChiliTag &src_tag, ofVec2f &scale)
    {
      ChiliTag t;
      t.id = src_tag.id; 

      ofVec2f center_t;
      ofVec2f center_s = src_tag.center_n * scale;
      transform_pts(center_s, center_t);
      t.center = center_t;

      t.center_n.set(t.center / scale);

      vector<ofVec2f> corners_t;
      for (int j = 0; j < src_tag.corners_n.size(); j++)
        corners_t.push_back(src_tag.corners_n[j] * scale);
      transform_pts(corners_t);  
      t.corners = corners_t;

      for ( int i = 0; i < t.corners.size(); i++ )
        t.corners_n.push_back(t.corners[i] / scale);

      t.dir.set(t.corners_n[0] - t.corners_n[1]);
      t.dir.normalize();
      t.angle = t.dir.angleRad(UP) + PI; 

      return t;
    }; 

    void undistort(vector<ofVec2f>& src, vector<ofVec2f>& dst)
    {
      if (!cam_calib.isReady())
        return;
      cam_calib.undistort(src, dst);
    };

    ofVec2f undistort(ofVec2f &src)
    {
      if (!cam_calib.isReady())
        return src;
      return cam_calib.undistort(src);
    };

    ofVec2f tag_from_proj_coord(ofVec2f& proj_coord, ofVec2f& ctr, vector<ChiliTag>& tags)
    { 
      for (int i = 0; i < tags.size(); i++)
      {
        ofVec2f t = tags[i].center_n;
        ofVec2f d = t - ctr;
        if (equal_sign(d, proj_coord))
          return t;
      }
    }; 

    ofVec2f tags_ctr(vector<ChiliTag>& tags)
    {
      ofVec2f ctr(0,0);
      for (int i = 0; i < tags.size(); i++)
        ctr += tags[i].center_n; 
      ctr /= tags.size();
      return ctr;
    };

    bool equal_sign(ofVec2f& a, ofVec2f& b)
    {
      ofVec2f s = a * b;
      return s.x > 0 && s.y > 0;
    }; 

    void render_calib_tags()
    {
      float s = 80;
      for (int i = 0; i < proj_pts.size(); i++)
      {
        cv::Point2f& p = proj_pts[i];
        calib_tags[i].draw(p.x-s/2, p.y-s/2, s, s);
      }
    };

    void render_calib_pts()
    {
      ofPushStyle();
      ofSetColor(ofColor::green);
      ofSetLineWidth(2);
      float s = 20;
      for (int i = 0; i < proj_pts.size(); i++)
      {
        cv::Point2f& p = proj_pts[i];
        ofDrawLine( p.x, p.y-s, p.x, p.y+s );
        ofDrawLine( p.x-s, p.y, p.x+s, p.y );
      }
      ofPopStyle();
    };  

    bool save_H()
    {
      cv::FileStorage fs( ofToDataPath(H_cam_proj_file, false), cv::FileStorage::WRITE );
      fs << "homography" << H_cv;
      return true;
    };

    bool load_H()
    {
      cv::FileStorage fs( ofToDataPath(H_cam_proj_file, false), cv::FileStorage::READ ); 
      if ( !fs.isOpened() )
        return false;
      fs["homography"] >> H_cv;
      H_ready = true;
      H_maps_ready = false;
      return true;
    }; 

    bool load_cam_calib()
    {
      cv::FileStorage fs( ofToDataPath(calib_cam_file, false), cv::FileStorage::READ ); 

      if ( !fs.isOpened() )
        return false;

      //cam_calib.load(calib_cam_file, false);

      cv::Size imageSize;
      cv::Size2f sensorSize;
      cv::Mat cameraMatrix;
      cv::Mat distCoeffs;
      float reprojectionError;

      fs["cameraMatrix"] >> cameraMatrix;
      fs["imageSize_width"] >> imageSize.width;
      fs["imageSize_height"] >> imageSize.height;
      fs["sensorSize_width"] >> sensorSize.width;
      fs["sensorSize_height"] >> sensorSize.height;
      fs["distCoeffs"] >> distCoeffs;
      fs["reprojectionError"] >> reprojectionError; 

      double k1 = distCoeffs.at<double>(0);
      double k2 = distCoeffs.at<double>(1);
      double p1 = distCoeffs.at<double>(2);
      double p2 = distCoeffs.at<double>(3);
      double k3 = distCoeffs.at<double>(4);
      cam_calib.setDistortionCoefficients(k1, k2, p1, p2, k3);

      ofxCv::Intrinsics intrinsics;
      intrinsics.setup(cameraMatrix, imageSize, sensorSize);
      cam_calib.setIntrinsics(intrinsics);

      return true;
    };


    //GPU perf optim

    //gpgpu::Process H_proc;
    //ofTexture H_tex;

    //TODO perf calib.transform_tex: apply homography on GPU
    //void transform_tex(ofTexture& src, ofPixels& dst)
    //{
      //if (!H_ready)
        //return;
      //H_proc
        //.set("tex", src)
        //.update();
      //H_tex = H_proc.get();
      ////XXX FIXME perf bottleneck (calib.transform_tex): read pixels GPU -> CPU
      //dst = H_proc.get_data_pix(); 
    //};

    //void init_H_gpu()
    //{
      //H_proc
        //.init("glsl/cv/homography.frag", proj_w, proj_h)
        //.on("update", this, &Calib::update_H_proc);
    //};

    //void dispose_H_gpu()
    //{
      //H_proc
        //.off("update", this, &Calib::update_H_proc)
        //.dispose();
    //};

    //void update_H_proc(ofShader& shader)
    //{
      ////see ofShader.setUniformMatrix3f
      //if (!shader.isLoaded()) return;
      //int loc = shader.getUniformLocation("H_inverse");
      //if (loc == -1) return;
      //int count = 1;
      //float* H_cv_ptr = H_cv.ptr<float>(); 
      //bool transpose = true;
      //glUniformMatrix3fv(loc, count, transpose, H_cv_ptr);
    //};
};

