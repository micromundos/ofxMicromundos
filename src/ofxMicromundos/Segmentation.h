#pragma once

#include "ofxCv.h"
#include "ofxChilitags.h"
#include "ofxMicromundos/utils.h"
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
      out_pix.allocate(w, h, 1);
      out_pix_back.allocate(w, h, 1);
      out_pix_render.allocate(w, h, 1);
      if (threaded) 
        startThread();
    };

    void update(ofPixels& pix, vector<ChiliTag>& tags)
        //, float w, float h)
    {
      if (threaded)
      {
        lock();
        front_pix = pix;
        front_tags = tags;
        new_data = true;
        //width = w;
        //height = h;
        if (segmented)
        {
          swap(out_pix, out_pix_intra);
          update_tex();
          segmented = false;
        }
        condition.signal();
        unlock();
      }
      else
      {
        //width = w;
        //height = h;
        segment(pix, tags, out_pix);
        update_tex();
      }
    };

    void render(float x, float y, float w, float h)
    {
      if (out_tex.isAllocated())
        out_tex.draw(x, y, w, h);
    };

    void dispose()
    {
      out_pix.clear();
      out_pix_back.clear();
      out_pix_intra.clear();
      out_pix_render.clear();
      out_tex.clear();
      back_pix.clear();
      front_pix.clear();
    };

    ofPixels& pixels()
    {
      return out_pix;
    };

  private:

    ofPixels out_pix, out_pix_back, out_pix_intra, out_pix_render;
    ofTexture out_tex;
    cv::Mat bin_mat;

    //float width, height;

    bool threaded, new_data, segmented;
    ofPixels back_pix, front_pix;
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
        {
          segment(back_pix, back_tags, out_pix_back);
          lock();
          swap(out_pix_back, out_pix_intra);
          segmented = true;
          unlock();
        }
      }
    };

    void segment(ofPixels& pix, vector<ChiliTag>& tags, ofPixels& dst)
    {
      TS_START("segment");

      //TODO perf: (segment) get gray pix from previous task (chilitags?)
      ofxCv::copyGray(pix, bin_mat);

      //ofxCv::resize(bin_mat, bin_mat, width/pix.getWidth(), height/pix.getHeight());

      ofxCv::autothreshold(bin_mat, false);

      //TODO perf: (segment) fill_tags -> move to gpu
      fill_tags(tags, bin_mat);

      //TODO perf: (segment) dilate/erode -> move to gpu
      //open
      //ofxCv::erode(bin_mat, 2);
      //ofxCv::dilate(bin_mat, 2);
      //close
      //ofxCv::dilate(bin_mat, 2);
      //ofxCv::erode(bin_mat, 4);

      ofxCv::toOf(bin_mat, dst); 

      TS_STOP("segment");
    };

    void update_tex()
    {
      ofxMicromundos::copy_pix(out_pix, out_pix_render);
      out_tex.loadData(out_pix_render);
    };

    //based on ofxCv::fillPoly(points, dst);
    void fill_tags(vector<ChiliTag>& tags, cv::Mat dstMat)
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

