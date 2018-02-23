#pragma once

#include "BinClient.h"
#include "MsgClient.h"

class BackendClient
{
  public:

    BackendClient() {};
    ~BackendClient() 
    {
      dispose();
    };

    void init(
        string ip, 
        int port_bin, 
        int port_msg, 
        float proj_w, 
        float proj_h,
        cv::FileNode _proj_pts)
    {
      this->proj_w = proj_w;
      this->proj_h = proj_h;
      init_calib_pts(_proj_pts);

      msg.init(ip, port_msg); 
      bin.init(ip, port_bin);
    };

    void update()
    {
      msg.update();
      if (msg.pix_ready()) 
        bin.update(msg.pix_width(), msg.pix_height(), msg.pix_chan());
    };

    void dispose()
    {
      msg.dispose();
      bin.dispose();
    };

    bool render_calib(float w, float h)
    {
      if (calib_enabled())
        render_calib_pts();
      return calib_enabled();
    };

    void render_projected_pixels(float w, float h)
    {
      bin.render(0, 0, w, h);
    };

    void print_connection(float x, float y)
    {
      float lh = 24;
      bin.print_connection(x, y);
      msg.print_connection(x, y + lh);
    };

    void print_metadata(float x, float y)
    {
      msg.print_metadata(x, y);
    };

    void print_bloques(float x, float y)
    {
      msg.print_bloques(x, y);
    };

    ofPixels& projected_pixels()
    {
      return bin.pixels();
    };

    //ofTexture& projected_texture()
    //{
      //return bin.texture();
    //};

    map<int, Bloque>& projected_bloques()
    {
      return msg.bloques();
    };

    bool calib_enabled()
    {
      return msg.calib_enabled();
    };

  private:

    BinClient bin;
    MsgClient msg;

    float proj_w, proj_h;
    vector<cv::Point2f> proj_pts;

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

    void init_calib_pts(cv::FileNode _proj_pts)
    {
      float w = proj_w;
      float h = proj_h;
      vector<vector<float>> pp;
      _proj_pts >> pp;
      proj_pts.push_back(cv::Point2f( w*pp[0][0], h*pp[0][1] ));
      proj_pts.push_back(cv::Point2f( w*pp[1][0], h*pp[1][1] ));
      //XXX clockwise
      proj_pts.push_back(cv::Point2f( w*pp[3][0], h*pp[3][1] ));
      proj_pts.push_back(cv::Point2f( w*pp[2][0], h*pp[2][1] ));
    };
};

