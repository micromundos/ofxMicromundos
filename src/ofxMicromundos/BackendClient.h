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
        ofxJSON pp)
    {
      this->proj_w = proj_w;
      this->proj_h = proj_h;
      init_calib_pts(pp);

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

    string juego_active() 
    { 
      return msg.juego_active(); 
    };

    bool juego_active(string name) 
    { 
      return msg.juego_active(name); 
    };

    bool calib_enabled() { return msg.calib_enabled(); };
    bool syphon_enabled() { return msg.syphon_enabled(); }; 
    //int pix_width() { return msg.pix_width(); };
    //int pix_height() { return msg.pix_height(); };
    //int pix_ready() { return msg.pix_ready(); }; 

  private:

    BinClient bin;
    MsgClient msg;

    float proj_w, proj_h;
    vector<ofVec2f> proj_pts;

    void render_calib_pts()
    {
      ofPushStyle();
      ofSetColor(ofColor::green);
      ofSetLineWidth(2);
      float s = 20;
      for (int i = 0; i < proj_pts.size(); i++)
      {
        ofVec2f& p = proj_pts[i];
        ofDrawLine( p.x, p.y-s, p.x, p.y+s );
        ofDrawLine( p.x-s, p.y, p.x+s, p.y );
      }
      ofPopStyle();
    };

    void init_calib_pts(ofxJSON pp)
    {
      float w = proj_w;
      float h = proj_h;

      //vector<vector<float>> pp;
      //_proj_pts >> pp;

      proj_pts.push_back(ofVec2f( 
            w*pp[0][0].asFloat(), h*pp[0][1].asFloat() ));
      proj_pts.push_back(ofVec2f( 
            w*pp[1][0].asFloat(), h*pp[1][1].asFloat() ));
      //XXX clockwise
      proj_pts.push_back(ofVec2f( 
            w*pp[3][0].asFloat(), h*pp[3][1].asFloat() ));
      proj_pts.push_back(ofVec2f( 
            w*pp[2][0].asFloat(), h*pp[2][1].asFloat() ));
    };
};

