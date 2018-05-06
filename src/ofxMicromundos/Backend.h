#pragma once

#include "ofxTimeMeasurements.h"
#include "ofxChilitags.h"
#include "ofxMicromundos/RGB.h"
#include "ofxMicromundos/net/ws/MsgServer.h"
#include "ofxMicromundos/net/ws/BinServer.h"
#include "ofxMicromundos/net/ws/BlobsServer.h"
#include "ofxMicromundos/Calib.h"
#include "ofxMicromundos/Segmentation.h"
#include "ofxMicromundos/Bloque.h"
#include "ofxMicromundos/Juegos.h"
#include "ofxMicromundos/Blobs.h"

//TODO Backend with ofThread
//TODO Backend dedupe print funcs (MsgClient, etc)

class Backend
{
  public:

    Backend() {};
    ~Backend() 
    {
      dispose();
    };

    void init(
        float proj_w, 
        float proj_h, 
        float cam_w, 
        float cam_h, 
        int cam_device_id, 
        string calib_H_cam_proj_file, 
        string calib_cam_file, 
        float resize_bin,
        int calib_tag_id,
        const Json::Value& juegos_config,
        int port_bin,
        int port_msg,
        int port_blobs)
    {
      this->proj_w = proj_w;
      this->proj_h = proj_h;
      this->resize_bin = resize_bin;

      _calib_enabled = false;
      _updated = false;

      calib.init(
          proj_w, proj_h, 
          calib_H_cam_proj_file, 
          calib_cam_file, 
          calib_tag_id);

      cam.init(cam_w, cam_h, cam_device_id);
      chilitags.init(); 
      seg.init();
      blobs.init();

      msg_server.init(port_msg);
      bin_server.init(port_bin);
      blobs_server.init(port_blobs);

      juegos.init(juegos_config);
    };

    bool update()
    {
      TS_START("cam");
      _updated = cam.update();
      TS_STOP("cam");

      if (!_updated)
        return false;

      cam_pix = cam.pixels();
      TS_START("undistort");
      calib.undistort(cam_pix);
      TS_STOP("undistort");

      TS_START("chilitags_copy");
      copy(cam_pix, chili_pix);
      TS_STOP("chilitags_copy");

      TS_START("chilitags");
      chilitags.update(chili_pix);
      TS_STOP("chilitags");

      vector<ChiliTag>& tags = chilitags.tags();

      _calib_enabled = calib.enabled(tags);
      if (_calib_enabled)
        calib.calibrate(tags, proj_w, proj_h);

      //ofPixels seg_pix_in;
      //ofxCv::resize(cam_pix, seg_pix_in, resize_bin, resize_bin);
      TS_START("segmentation");
      seg.update(cam_pix, tags); 
      TS_STOP("segmentation");

      ofPixels seg_pix;

      TS_START("segmentation_copy");
      copy(seg.pixels(), seg_pix);
      TS_STOP("segmentation_copy");

      TS_START("transform_pix");
      calib.transform(seg_pix, proj_pix, proj_w, proj_h);
      TS_STOP("transform_pix");

      proj_tex.loadData(proj_pix);

      TS_START("transform_tags");
      calib.transform(tags, proj_tags, proj_w, proj_h);
      TS_STOP("transform_tags");

      TS_START("tags_to_bloques");
      tags_to_bloques(proj_tags, proj_bloques);
      TS_STOP("tags_to_bloques");

      TS_START("blobs");
      if (blobs_server.connected())
        blobs.update(proj_pix);
      TS_STOP("blobs");

      juegos.update(proj_bloques);

      return true;
    };

    void send(
        bool message_enabled, 
        bool binary_enabled, 
        bool syphon_enabled,
        bool blobs_enabled)
    {
      if (!_updated) 
        return false;

      ofPixels* out_pix;
      TS_START("resize_to_send");
      if (resize_bin != 1.0)
      {
        ofxCv::resize(
            proj_pix, proj_pix_resized, 
            resize_bin, resize_bin);
        out_pix = &proj_pix_resized;
      }
      else
        out_pix = &proj_pix;
      TS_STOP("resize_to_send");

      TS_START("send_msg");
      msg_server.send(
          *out_pix,
          proj_bloques,
          message_enabled,
          binary_enabled,
          syphon_enabled,
          _calib_enabled,
          juegos.active());
      TS_STOP("send_msg");

      TS_START("send_bin");
      bin_server.send(*out_pix, binary_enabled);
      TS_STOP("send_bin");

      TS_START("send_blobs");
      blobs_server.send(blobs.get(), blobs_enabled);
      TS_STOP("send_blobs");
    };

    bool render_calib(float w, float h)
    {
      if (_calib_enabled)
      {
        calib.render();
        render_projected_tags();
      }
      return _calib_enabled;
    };

    void render_projected_pixels(float w, float h)
    {  
      if (proj_tex.isAllocated())
        proj_tex.draw(0, 0, w, h);
    };

    void render_projected_tags()
    {
      ofPushStyle();
      ofSetColor(ofColor::orange);
      ofSetLineWidth(5);
      for (int i = 0; i < proj_tags.size(); i++)
      {
        vector<ofVec2f> &corners = proj_tags[i].corners;
        ofVec2f p0, p1;
        for (int j = 0; j < corners.size(); j++)
        {
          p0 = corners[j];
          p1 = corners[ (j+1)%4 ]; 
          ofDrawLine( p0.x, p0.y, p1.x, p1.y );
        }
      }
      ofPopStyle();
    };

    void render_monitor(float x, float y, float w, float h)
    {
      float _h = h/3;

      cam.render(x, y, w, _h);

      if (cam_pix.isAllocated())
        cam_tex.loadData(cam_pix);
      if (cam_tex.isAllocated())
        cam_tex.draw(x, y+_h, w, _h);

      chilitags.render(x, y+_h, w, _h);

      seg.render(x, y+_h*2, w, _h);

      if (blobs_server.connected())
        blobs.render(x, y+_h*2, w, _h);
    };

    void print_connection(float x, float y)
    { 
      print_ws_connection(
          msg_server.server(), 
          msg_server.connected(), 
          "msg", x, y);
      print_ws_connection(
          bin_server.server(), 
          bin_server.connected(), 
          "bin", x, y);
      print_ws_connection(
          blobs_server.server(), 
          blobs_server.connected(), 
          "blobs", x, y);
    }; 

    void print_metadata(float x, float y)
    {
      stringstream status;
      status << "metadata= \n";

      if (proj_pix.isAllocated())
      {
        status << " pixels:" 
            << " dim " 
              << proj_pix.getWidth() << "," 
              << proj_pix.getHeight()
            << " chan " << proj_pix.getNumChannels()
          << "\n";
      }

      status << " calib:" 
          << " enabled " << _calib_enabled
        << "\n"
        << " juegos:"
          << " active " << juegos.active();

      float lh = 24;
      ofDrawBitmapStringHighlight(status.str(), x, y+lh/2);
    };

    void print_bloques(float x, float y)
    {
      float lh = 24;
      y += lh/2;
      ofDrawBitmapStringHighlight("bloques", x, y, ofColor::yellow, ofColor::black);
      y += lh;
      for (const auto& bloque : proj_bloques)
      {
        const Bloque& b = bloque.second;
        stringstream status;
        status
          << " id " << b.id
          << " loc " << b.loc;
          //<< " dir " << b.dir
          //<< " radio " << b.radio
          //<< " angle " << b.angle;
        ofDrawBitmapStringHighlight(status.str(), x, y, ofColor::yellow, ofColor::black);
        y += lh;
      }
    }; 

    void print_blobs(float x, float y)
    {
      float lh = 24;
      y += lh/2;
      string status = !blobs_server.connected() ? "not connected" : ofToString(blobs.get().size()); 
      ofDrawBitmapStringHighlight("blobs: "+status, x, y, ofColor::yellow, ofColor::black);
    };

    void dispose()
    {
      cam.dispose();
      calib.dispose();
      seg.dispose();
      blobs.dispose();
      chili_pix.clear();
      cam_pix.clear();
      cam_tex.clear();
      proj_pix.clear();
      proj_pix_resized.clear();
      proj_tex.clear(); 
      proj_tags.clear();
      proj_bloques.clear();
      msg_server.dispose();
      bin_server.dispose();
      blobs_server.dispose();
    };

    ofPixels& projected_pixels()
    {
      return proj_pix;
    };

    ofTexture& projected_texture()
    {
      return proj_tex;
    };

    map<int, Bloque>& projected_bloques()
    {
      return proj_bloques;
    };

    bool calib_enabled()
    {
      return _calib_enabled;
    };

  private:

    float proj_w, proj_h;
    float resize_bin;
    float _calib_enabled;
    bool _updated;

    RGB cam;
    MsgServer msg_server;
    BinServer bin_server;
    BlobsServer blobs_server;
    Calib calib;
    Segmentation seg;
    Blobs blobs;
    ofxChilitags chilitags;
    Juegos juegos;

    ofPixels cam_pix;
    ofPixels chili_pix;
    ofTexture cam_tex;
    ofPixels proj_pix;
    ofPixels proj_pix_resized;
    ofTexture proj_tex;

    vector<ChiliTag> proj_tags;
    map<int, Bloque> proj_bloques; 


    void tags_to_bloques(vector<ChiliTag>& tags, map<int, Bloque>& bloques)
    {
      map<int,bool> cur;

      for (int i = 0; i < tags.size(); i++)
      {
        int id = tags[i].id;
        cur[id] = true;
        if (bloques.find(id) == bloques.end())
          make_bloque(tags[i], bloques);
        else 
          update_bloque(tags[i], bloques[id]);
      }

      vector<int> remove;
      for (const auto& bloque : bloques)
      {
        int id = bloque.first;
        if (cur.find(id) == cur.end())
          remove.push_back(id);
      }
      for (const auto& id : remove)
        bloques.erase(id);
    };

    void make_bloque(ChiliTag& t, map<int, Bloque>& bloques)
    {
      Bloque b;

      set_bloque(t, b);

      b.loc_i = b.loc;
      b.dir_i = b.dir;
      b.angle_i = b.angle;

      bloques[b.id] = b;
    };

    void update_bloque(ChiliTag& t, Bloque& b)
    {
      interpolate_bloque(t, b);
      set_bloque(t, b); 
    };

    void set_bloque(ChiliTag& t, Bloque& b)
    {
      b.id = t.id;
      b.loc = t.center_n;
      b.dir = t.dir;
      b.angle = t.angle;

      ofVec2f corner(t.corners_n[0].x, t.corners_n[0].y);
      b.radio = b.loc.distance(corner);
      //b.corners = t.corners_n;
    };

    void interpolate_bloque(ChiliTag& t, Bloque& b)
    {
      b.loc_i += (t.center_n - b.loc_i) * 0.2;
      b.dir_i += (t.dir - b.dir_i) * 0.2;
      b.angle_i = ofLerpRadians(b.angle_i, t.angle, 0.05);
    };

    void copy(ofPixels& src, ofPixels& dst)
    {
      dst.setFromPixels(src.getData(), src.getWidth(), src.getHeight(), src.getNumChannels());
    };

    void print_ws_connection(
        ofxLibwebsockets::Server& server, 
        bool connected, 
        string name, 
        float x, float& y)
    {
      float lh = 24; 

      if (!connected)
      {
        y += lh;
        ofDrawBitmapStringHighlight("ws server "+name+" not connected", x, y, ofColor::red, ofColor::black);
        return;
      } 

      ofDrawBitmapStringHighlight("ws server "+name+" port: "+ofToString(server.getPort()), x, y, ofColor::green, ofColor::black);

      vector<ofxLibwebsockets::Connection*> conns = server.getConnections();
      for (int i = 0; i < conns.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns[i];

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from ip "+ip;

        y += lh;
        ofDrawBitmapString(info, x, y);
      }
    };
};

