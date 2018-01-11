#pragma once

#include "ofxCv.h"
#include "ofxChilitags.h"

using namespace ofxCv;

class Calib
{
  public:

    Calib() {};
    ~Calib() 
    {
      dispose();
    };

    bool init(float w, float h)
    {
      calib_tag_id = 0;
      calib_file = "calib/H_rgb_proj.yml";
      UP = ofVec2f(0,1);
      H_ready = false;

      calib_tags.resize(4);
      for (int i = 0; i < calib_tags.size(); i++)
        calib_tags[i].load("calib/"+ofToString(i+1)+".png");

      //float s = 50.;
      //float cx = w/2;
      //float cy = h/2;

      proj_pts.push_back(cv::Point2f( w*1./3., h*1./3. ));
      proj_pts.push_back(cv::Point2f( w*2./3., h*1./3. ));
      proj_pts.push_back(cv::Point2f( w*2./3., h*2./3. ));
      proj_pts.push_back(cv::Point2f( w*1./3., h*2./3. ));

      proj_coords.push_back(ofVec2f( -1.,-1. ));
      proj_coords.push_back(ofVec2f(  1.,-1. ));
      proj_coords.push_back(ofVec2f(  1., 1. ));
      proj_coords.push_back(ofVec2f( -1., 1. ));

      load();
    };

    bool enabled(vector<ChiliTag>& tags)
    {
      for (int i = 0; i < tags.size(); i++)
        if (is_calib_tag(tags[i]))
          return true;
      return false;
    };

    void find(vector<ChiliTag>& _tags, float w, float h)
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

      save();
    }; 

    //TODO calib render tags to proj
    void render()
    { 
      render_calib_tags();
      render_calib_pts();
    }; 

    void transform(ofPixels &src, ofPixels &dst, float w, float h)
    {
      if (!H_ready)
        return;

      float sw = src.getWidth();
      float sh = src.getHeight(); 

      ofPixels src_t; 
      //ofxCv::imitate(src_t, src);
      ofxCv::copy(src, src_t);
      ofxCv::resize(src, src_t, w/sw, h/sh);
      _transform(src_t, dst);
    }; 

    //in place
    void transform(ofPixels &pix, float w, float h) 
    { 
      transform(pix, pix, w, h); 
    };

    void transform(vector<ChiliTag>& src_tags, vector<ChiliTag>& dst_tags, float w, float h)
    {
      if (!H_ready)
        return;

      ofVec2f scale(w,h); 

      dst_tags.clear();
      for (int i = 0; i < src_tags.size(); i++)
        dst_tags.push_back(transform_tag(src_tags[i], scale)); 
    };

    void dispose()
    {}; 

  private:

    int calib_tag_id;
    cv::Mat H_cv;
    bool H_ready;
    ofVec2f UP;
    string calib_file;
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

    void _transform(const ofVec2f &src, ofVec2f &dst)
    {
      vector<ofVec2f> srcPts;
      srcPts.push_back(src);
      vector<ofVec2f> dstPts;
      dstPts.push_back(dst);
      _transform(srcPts, dstPts);
    }; 

    void _transform(vector<ofVec2f> &src, vector<ofVec2f> &dst)
    {
      vector<cv::Point2f> srcPts = toCv(src);
      vector<cv::Point2f> dstPts(src.size());
      cv::perspectiveTransform(srcPts, dstPts, H_cv);
      dst.resize(src.size());
      for(int i = 0; i < dstPts.size(); i++) 
      {
        dst[i].x = dstPts[i].x;
        dst[i].y = dstPts[i].y;
      }
    };

    void _transform(vector<cv::Point2f> &src, vector<cv::Point2f> &dst)
    {
      cv::perspectiveTransform(src, dst, H_cv);
    };

    void _transform(ofPixels& src, ofPixels& dst)
    {
      cv::Mat srcMat = toCv(src);
      cv::Mat dstMat = toCv(dst);
      cv::warpPerspective(srcMat, dstMat, H_cv, srcMat.size(), cv::INTER_LINEAR);
      toOf(dstMat, dst);
    };

    void _transform(ofVec2f &point)
    {
      _transform(point, point);
    };

    void _transform(vector<ofVec2f> &points)
    {
      _transform(points, points);
    };

    void _transform(ofPixels& pix)
    {
      _transform(pix, pix);
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

    ChiliTag transform_tag(ChiliTag &src_tag, ofVec2f &scale)
    {
      ChiliTag t;
      t.id = src_tag.id; 

      ofVec2f center_t;
      _transform(src_tag.center_n * scale, center_t);
      t.center = center_t;

      t.center_n.set(t.center / scale);

      vector<ofVec2f> corners_t;
      for (int j = 0; j < src_tag.corners_n.size(); j++)
        corners_t.push_back(src_tag.corners_n[j] * scale);
      _transform(corners_t);  
      t.corners = corners_t;

      for ( int i = 0; i < t.corners.size(); i++ )
        t.corners_n.push_back(t.corners[i].x / scale);

      t.dir.set(t.corners_n[0] - t.corners_n[1]);
      t.dir.normalize();
      t.angle = t.dir.angleRad(UP) + PI; 

      return t;
    };

    void render_calib_tags()
    {
      float s = 60;
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
      float s = 5;
      for (int i = 0; i < proj_pts.size(); i++)
      {
        cv::Point2f& p = proj_pts[i];
        ofDrawLine( p.x, p.y-s, p.x, p.y+s );
        ofDrawLine( p.x-s, p.y, p.x+s, p.y );
      }
      ofPopStyle();
    };  

    bool save()
    {
      cv::FileStorage fs( ofToDataPath(calib_file, false), cv::FileStorage::WRITE );
      fs << "homography" << H_cv;
      return true;
    };

    bool load()
    {
      cv::FileStorage fs( ofToDataPath(calib_file, false), cv::FileStorage::READ ); 
      if ( !fs.isOpened() )
        return false;
      fs["homography"] >> H_cv;
      H_ready = true;
      return true;
    }; 
};

