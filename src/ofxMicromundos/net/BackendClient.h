#pragma once

#include "ofxMicromundos/utils.h"
#include "ofxMicromundos/net/ws/BinClient.h"
#include "ofxMicromundos/net/ws/MsgClient.h"
#include "ofxMicromundos/net/ws/BlobsClient.h"
#include "ofxMicromundos/BlobsMesh.h"
#include "ofxMicromundos/Blobs.h"

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
        float proj_h)
    {
      proj_pts = ofxMicromundos::calib_points(proj_w, proj_h);
      _msg.init(ip, port_msg); 
      _bin.init(ip, port_bin);
      _blobs.init(ip, port_blobs); 
      LH = 24; 
    };

    void update()
    {
      _msg.update();
      if (_msg.pix_ready()) 
      {
        _bin.update(_msg.pix_width(), _msg.pix_height(), _msg.pix_chan());
        _blobs.update();
        _blobs_mesh.update(_blobs.get(), ofFloatColor::white);
      }
    };

    void dispose()
    {
      _msg.dispose();
      _bin.dispose();
      _blobs.dispose();
      _blobs_mesh.dispose();
    };

    bool render_calib()
    {
      if (calib_enabled())
        render_calib_pts();
      return calib_enabled();
    };

    void render_texture(float x, float y, float w, float h)
    {
      _bin.render(x, y, w, h);
    };

    void render_blobs(float x, float y, float w, float h)
    {
      _blobs_mesh.render(x, y, w, h);
      //Blobs::render_debug(_blobs.get(), x, y, w, h);
    };

    void print_connections(float x, float& y)
    {
      _bin.print_connection(x, y, LH);
      _msg.print_connection(x, y, LH);
      _blobs.print_connection(x, y, LH);
    };

    void print_metadata(float x, float& y)
    {
      _msg.print_metadata(x, y, LH);
    };

    void print_bloques(float x, float& y)
    {
      _msg.print_bloques(x, y, LH);
    };

    ofPixels& pixels()
    {
      return _bin.pixels();
    };

    ofTexture& texture()
    {
      return _bin.texture();
    };

    map<int, Bloque>& bloques()
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

    float line_height() { return LH; };

  private:

    BinClient _bin;
    MsgClient _msg;
    BlobsClient _blobs;
    BlobsMesh _blobs_mesh;
    vector<ofVec2f> proj_pts;
    float LH;

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

};

