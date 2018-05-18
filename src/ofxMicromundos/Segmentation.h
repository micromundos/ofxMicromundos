#pragma once

#include "ofxCv.h"
#include "ofxChilitags.h"
#include "Poco/Condition.h"

class Segmentation : public ofThread
{
  public:

    Segmentation() {};
    ~Segmentation() 
    {
      dispose();
    };

    void init(float w, float h, bool threaded = true) 
    {
      this->threaded = threaded;
      bin_pix.allocate(w, h, 1);
      if (threaded) 
        startThread();
    };

    void update(ofPixels& pix, vector<ChiliTag>& tags)
    {
      if (threaded)
      {
        lock();
        front_pix = pix;
        front_tags = tags;
        new_data = true;
        if (segmented)
        {
          swap(bin_pix, intra_pix);
          segmented = false;
        }
        condition.signal();
        unlock();
      }
      else
      {
        segment(pix, tags);
      }
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

    ofPixels& pixels()
    {
      return bin_pix;
    };

  private:

    ofPixels bin_pix;
    ofTexture bin_tex;

    bool threaded, new_data, segmented;
    ofPixels back_pix, front_pix, intra_pix;
    vector<ChiliTag> back_tags, front_tags;
    Poco::Condition condition;

    void threadedFunction()
    {
      while (isThreadRunning())
      {
        lock();
        if (!new_data)
          condition.wait(mutex);
        bool run_segmentation = false;
        if (new_data)
        {
          swap(front_pix, back_pix);
          swap(front_tags, back_tags);
          run_segmentation = true;
          new_data = false;
        }
        unlock();
        if (run_segmentation)
          segment(back_pix, back_tags);
      }
    };

    void segment(ofPixels& pix, vector<ChiliTag>& tags)
    {
      cv::Mat bin_mat;
      ofxCv::copyGray(pix, bin_mat);
      ofxCv::autothreshold(bin_mat, false);
      fillTags(tags, bin_mat);

      //open
      ofxCv::erode(bin_mat, 2);
      ofxCv::dilate(bin_mat, 2);
      //close
      ofxCv::dilate(bin_mat, 2);
      ofxCv::erode(bin_mat, 4);

      //ofxCv::toOf(bin_mat, bin_pix); 
      ofxCv::toOf(bin_mat, back_pix); 

      if (threaded)
      {
        lock();
        swap(back_pix, intra_pix);
        segmented = true;
        unlock();
      }
      else
      {
        swap(back_pix, bin_pix);
      }
    };

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

