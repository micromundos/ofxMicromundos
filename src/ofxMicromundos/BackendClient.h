#pragma once

#include "BinClient.h"
#include "MsgClient.h"
#include "BlobsClient.h"

//TODO move BackendClient to net/ws/
  //net/ws/MsgClient
  //net/ws/BinClient
  //net/ws/BlobsClient
//TODO BackendClient rename projected_xxx to xxx
//TODO BackendClient hardcode proj_pts/ofxJSON pp

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
        int port_blobs,
        float proj_w, 
        float proj_h,
        ofxJSON pp)
    {
      this->proj_w = proj_w;
      this->proj_h = proj_h;
      init_calib_pts(pp);

      _msg.init(ip, port_msg); 
      _bin.init(ip, port_bin);
      _blobs.init(ip, port_blobs); 
    };

    void update()
    {
      _msg.update();
      if (_msg.pix_ready()) 
      {
        _bin.update(_msg.pix_width(), _msg.pix_height(), _msg.pix_chan());
        _blobs.update();
      }
    };

    void dispose()
    {
      _msg.dispose();
      _bin.dispose();
      _blobs.dispose();
    };

    bool render_calib(float w, float h)
    {
      if (calib_enabled())
        render_calib_pts();
      return calib_enabled();
    };

    void render_projected_pixels(float w, float h)
    {
      _bin.render(0, 0, w, h);
    };

    void render_blobs(float x, float y, float w, float h)
    {
      ofPushMatrix();
      ofTranslate(x, y);
      ofScale(w, h); 
      for (const auto& blob : _blobs.get())
        blob.draw();
      ofPopMatrix();
    };

    void print_connection(float x, float y)
    {
      float lh = 24;
      _bin.print_connection(x, y);
      _msg.print_connection(x, y + lh);
      _blobs.print_connection(x, y + lh*2);
    };

    void print_metadata(float x, float y)
    {
      _msg.print_metadata(x, y);
    };

    void print_bloques(float x, float y)
    {
      _msg.print_bloques(x, y);
    };

    ofPixels& projected_pixels()
    {
      return _bin.pixels();
    };

    ofTexture& projected_texture()
    {
      return _bin.texture();
    };

    map<int, Bloque>& projected_bloques()
    {
      return _msg.bloques();
    };

    vector<ofPolyline>& blobs()
    {
      return _blobs.get();
    };

    string juego_active() 
    { 
      return _msg.juego_active(); 
    };

    bool juego_active(string name) 
    { 
      return _msg.juego_active(name); 
    };

    bool calib_enabled() { return _msg.calib_enabled(); };
    bool syphon_enabled() { return _msg.syphon_enabled(); }; 
    //int pix_width() { return _msg.pix_width(); };
    //int pix_height() { return _msg.pix_height(); };
    //int pix_ready() { return _msg.pix_ready(); }; 

  private:

    BinClient _bin;
    MsgClient _msg;
    BlobsClient _blobs;

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

