#pragma once

#include "ofxGPGPU.h"
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

      dilate
        .add_backbuffer("tex")
        .init("glsl/openvision/dilate.fs", w, h);

      erode
        .add_backbuffer("tex")
        .init("glsl/openvision/erode.fs", w, h);

      if (threaded) 
        startThread();
    };

    void update(
        ofPixels& pix, 
        vector<ChiliTag>& tags, 
        bool update_pix)
    {
      //option to update pixels on gpu mode for perf optimization
      this->update_pix = update_pix;

      if (threaded)
      {
        lock();
        front_pix = pix;
        front_tags = tags;
        new_data = true;
        if (segmented)
        {
          swap(out_pix, out_pix_intra);
          //XXX TODO update threaded segmentation results
          segmented = false;
        }
        condition.signal();
        unlock();
      }
      else
      {
        update_1_thread(pix, tags); 
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
      out_tex.clear();
      back_pix.clear();
      front_pix.clear();
      dilate.dispose();
      erode.dispose();
    };

    ofPixels& pixels()
    {
      return out_pix;
    };

    ofTexture& texture()
    {
      return out_tex;
    };

  private:

    ofPixels out_pix, out_pix_back, out_pix_intra;
    ofTexture out_tex;
    cv::Mat bin_mat;
    gpgpu::Process dilate;
    gpgpu::Process erode;
    bool update_pix;

    bool threaded, new_data, segmented;
    ofPixels back_pix, front_pix;
    vector<ChiliTag> back_tags, front_tags;
    Poco::Condition condition;

    void update_1_thread(ofPixels& pix, vector<ChiliTag>& tags)
    {
      segment(pix, tags, bin_mat);
      open_close_cpu(bin_mat);
      update_out_cpu(out_pix);
      //open_close_gpu(bin_mat);
      //update_out_gpu(out_pix);
    };

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
          segment(back_pix, back_tags, bin_mat);

          //XXX TODO update threaded segmentation results
          open_close_cpu(bin_mat);
          update_out_cpu(out_pix_back);
          //open_close_gpu(bin_mat);
          //update_out_gpu(out_pix_back);

          lock();
          swap(out_pix_back, out_pix_intra);
          segmented = true;
          unlock();
        }
      }
    };

    void segment(ofPixels& pix, vector<ChiliTag>& tags, cv::Mat& bin_mat)
    {
      TS_START("segment");
      //TODO perf (3a) (segment) use existing gray pix (from chilitags?)
      ofxCv::copyGray(pix, bin_mat);
      //TODO perf (3b) (segment) autothreshold -> move to gpu
      ofxCv::autothreshold(bin_mat, false);
      //TODO perf (3c) (segment) fill_tags -> move to gpu
      fill_tags(tags, bin_mat);
      TS_STOP("segment");
    };

    void update_out_gpu(ofPixels& dst)
    {
      out_tex = erode.get();
      //XXX casts float to unsigned char
      if (update_pix) dst = erode.get_data_pix(); 
    };

    void update_out_cpu(ofPixels& dst)
    {
      //always update pixels if using cpu
      ofxCv::toOf(bin_mat, dst); 
      out_tex.loadData(dst);
    };

    //TODO perf (3d) (segment) dilate/erode -> move to gpu
    void open_close_gpu(cv::Mat& bin_mat)
    {
      //FIXME pointer fails
      float* mat_ptr = bin_mat.ptr<float>(); 
      erode
        .set("tex", mat_ptr)
        .update(2);
      dilate
        .set("tex", erode.get())
        .update(4);
      erode
        .set("tex", dilate.get())
        .update(4); 
    };

    void open_close_cpu(cv::Mat& bin_mat)
    {
      //open
      ofxCv::erode(bin_mat, 2);
      ofxCv::dilate(bin_mat, 2);
      //close
      ofxCv::dilate(bin_mat, 2);
      ofxCv::erode(bin_mat, 4);
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

