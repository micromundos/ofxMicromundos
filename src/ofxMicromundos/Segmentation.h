#pragma once

//#include "ofxGPGPU.h"
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

      //dilate
        //.add_backbuffer("tex")
        //.init("glsl/openvision/dilate.fs", w, h);
      //erode
        //.add_backbuffer("tex")
        //.init("glsl/openvision/erode.fs", w, h);

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
          swap(out_pix, out_pix_intra);
          //XXX TODO update threaded segmentation results
          segmented = false;
        }
        condition.signal();
        unlock();
      }
      else
      {
        update_1_thread(pix, tags, out_pix);
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
      //dilate.dispose();
      //erode.dispose();
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

    ofPixels out_pix, out_pix_back, out_pix_intra, out_pix_render;
    ofTexture out_tex;
    cv::Mat bin_mat;

    //gpgpu::Process dilate;
    //gpgpu::Process erode;

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
          segment(back_pix, back_tags, bin_mat);

          //XXX TODO update threaded segmentation results
          filter_noise(bin_mat);
          update_output(bin_mat, out_pix_back);

          lock();
          swap(out_pix_back, out_pix_intra);
          segmented = true;
          unlock();
        }
      }
    };

    void update_1_thread(ofPixels& pix, vector<ChiliTag>& tags, ofPixels& out_pix)
    {
      segment(pix, tags, bin_mat);
      filter_noise(bin_mat);
      update_output(bin_mat, out_pix); 
    };

    void segment(ofPixels& pix, vector<ChiliTag>& tags, cv::Mat& bin_mat)
    {
      TS_START("segment");
      ofxCv::copyGray(pix, bin_mat);
      ofxCv::autothreshold(bin_mat, false);
      fill_tags(tags, bin_mat);
      TS_STOP("segment");
    };

    void filter_noise(cv::Mat& bin_mat)
    {
      TS_START("filter_noise");
      filter_noise_cpu(bin_mat);
      //filter_noise_gpu(bin_mat);
      TS_STOP("filter_noise");
    };

    void update_output(cv::Mat& bin_mat, ofPixels& dst)
    {
      TS_START("update_output");
      update_out_cpu(bin_mat, dst);
      //update_out_gpu(erode, out_pix);
      TS_STOP("update_output");
    };

    void filter_noise_cpu(cv::Mat& bin_mat)
    {
      //open
      ofxCv::erode(bin_mat, 2);
      ofxCv::dilate(bin_mat, 2);
      //close
      ofxCv::dilate(bin_mat, 2);
      ofxCv::erode(bin_mat, 4);
    }; 

    void update_out_cpu(cv::Mat& bin_mat, ofPixels& dst)
    {
      //always update pixels if using cpu
      ofxCv::toOf(bin_mat, dst); 
      out_tex.loadData(dst);
    };

    //TODO perf (3d) (segment) dilate/erode -> move to gpu
    //void filter_noise_gpu(cv::Mat& bin_mat)
    //{
      ////FIXME pointer fails
      //float* mat_ptr = bin_mat.ptr<float>(); 
      //erode
        //.set("tex", mat_ptr)
        //.update(2);
      //dilate
        //.set("tex", erode.get())
        //.update(4);
      //erode
        //.set("tex", dilate.get())
        //.update(4); 
    //};

    //void update_out_gpu(gpgpu::Process& proc, ofPixels& dst)
    //{
      //out_tex = proc.get();
      ////XXX casts float to unsigned char
      //if (update_pix) dst = proc.get_data_pix(); 
    //};

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

