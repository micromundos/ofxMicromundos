#pragma once

#include "ofxCv.h"
#include "ofxChilitags.h"

class Segmentation
{
  public:

    Segmentation() {};
    ~Segmentation() 
    {
      dispose();
    };

    void init() {};

    ofPixels& update(ofPixels& pix, vector<ChiliTag>& tags)
    {
      ofxCv::copyGray(pix, bin_mat);
      ofxCv::autothreshold(bin_mat, false);
      fillTags(tags, bin_mat);

      //open
      ofxCv::erode(bin_mat, 2);
      ofxCv::dilate(bin_mat, 2);
      //close
      ofxCv::dilate(bin_mat, 2);
      ofxCv::erode(bin_mat, 4);

      ofxCv::toOf(bin_mat, bin_pix); 

      return bin_pix;
    };

    void render(float x, float y, float w, float h)
    {
      if (bin_pix.isAllocated())
        bin_tex.loadData(bin_pix);
      if (bin_tex.isAllocated())
        bin_tex.draw(x, y, w, h);
    };

    void dispose()
    {
      bin_pix.clear();
      bin_tex.clear();
    };

    //ofPixels& pixels()
    //{
      //return bin_pix;
    //};

  private:

    cv::Mat bin_mat;
    ofPixels bin_pix;
    ofTexture bin_tex;

    //based on ofxCv::fillPoly(points, dst);
    void fillTags(vector<ChiliTag>& tags, cv::Mat dstMat)
    {
      int w = dstMat.cols;
      int h = dstMat.rows;

      for (int i = 0; i < tags.size(); i++)
      {
        vector<ofVec2f> &corners_n = tags[i].corners_n;
        vector<cv::Point> points(corners_n.size());
        for (int j = 0; j < corners_n.size(); j++)
        {
          points[j].x = corners_n[j].x * w;
          points[j].y = corners_n[j].y * h;
        } 

        const cv::Point* ppt[1] = { &(points[0]) };
        int npt[] = { (int) points.size() };
        cv::fillPoly(dstMat, ppt, npt, 1, cv::Scalar(255));
      }
    };

};

