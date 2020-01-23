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
#include "ofxMicromundos/utils.h"

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
        float chilitags_fps,
        float resize_seg,
        float resize_send,
        int calib_tag_id,
        const Json::Value& juegos_config,
        int port_bin,
        int port_msg,
        int port_blobs)
    {
      this->proj_w = proj_w;
      this->proj_h = proj_h;
      this->resize_seg = resize_seg;
      this->resize_send = resize_send;

      LH = 24; 
      _calib_enabled = false;
      cam_updated = false;
      send_pix = nullptr;

      proj_pix.allocate(proj_w, proj_h, 1);

      calib.init(
          proj_w, proj_h, 
          calib_H_cam_proj_file, 
          calib_cam_file, 
          calib_tag_id);

      cam.init(cam_w, cam_h, cam_device_id);
      chilitags.init(true, 10, chilitags_fps); 
      seg.init(cam_w, cam_h, false); //TODO segmentation thread
      blobs.init();

      msg_server.init(port_msg);
      bin_server.init(port_bin);
      blobs_server.init(port_blobs);

      juegos.init(juegos_config);
    };

    bool update()
    {
      TS_START("cam");
      cam_updated = cam.update();
      TS_STOP("cam");

      if (!cam_updated)
        return false;

      cam_pix = cam.pixels();

      //TS_START("undistort");
      //calib.undistort(cam_pix);
      //TS_STOP("undistort");

      TS_START("chilitags_copy");
      ofxMicromundos::copy_pix(cam_pix, chili_pix);
      TS_STOP("chilitags_copy");

      TS_START("chilitags");
      chilitags.update(chili_pix);
      TS_STOP("chilitags");

      vector<ChiliTag>& tags = chilitags.tags();

      _calib_enabled = calib.enabled(tags);
      if (_calib_enabled)
        calib.calibrate(tags, proj_w, proj_h);

      TS_START("segmentation_src");
      segmentation_src(cam_pix, seg_pix_src);
      TS_STOP("segmentation_src");

      TS_START("segmentation");
      seg.update(seg_pix_src, tags); 
      TS_STOP("segmentation");

      TS_START("segmentation_dst");
      segmentation_dst(seg, seg_pix_dst); 
      TS_STOP("segmentation_dst");

      TS_START("transform_pix");
      calib.transform_pix(seg_pix_dst, proj_pix);
      //calib.transform_pix(seg.pixels(), proj_pix);
      //calib.transform_tex(seg.texture(), proj_pix);
      TS_STOP("transform_pix");

      proj_tex.loadData(proj_pix);

      TS_START("transform_tags");
      calib.transform_tags(tags, proj_tags, proj_w, proj_h);
      TS_STOP("transform_tags");

      TS_START("tags_to_bloques");
      tags_to_bloques(proj_tags, proj_bloques);
      TS_STOP("tags_to_bloques");

      TS_START("blobs");
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
      if (!cam_updated) 
        return;

      TS_START("resize_to_send");
      if (resize_send != 1.0)
      {
        ofxMicromundos::resize(
            proj_pix, proj_pix_resized, 
            resize_send, resize_send);
        send_pix = &proj_pix_resized;
      }
      else
        send_pix = &proj_pix;
      TS_STOP("resize_to_send");

      TS_START("send_msg");
      msg_server.send(
          *send_pix,
          proj_bloques,
          message_enabled,
          binary_enabled,
          syphon_enabled,
          _calib_enabled,
          juegos.active());
      TS_STOP("send_msg");

      TS_START("send_bin");
      bin_server.send(*send_pix, binary_enabled);
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
        render_tags();
      }
      return _calib_enabled;
    };

    void render_tags()
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
      float _h = h/4;
      float _y = y;

      _y += LH/2;
      text("cam input", x, _y);
      _y += LH/2;
      cam.render(x, _y, w, _h);
      _y += _h;

      _y += LH;
      text("cam undistorted w/chilis", x, _y);
      _y += LH/2;
      load_render(cam_pix, cam_tex, x, _y, w, _h);
      chilitags.render(x, _y, w, _h);
      _y += _h;

      _y += LH;
      text("segmented", x, _y);
      _y += LH/2;
      seg.render(x, _y, w, _h);
      _y += _h;

      _y += LH;
      text("H transformed w/blobs", x, _y);
      _y += LH/2;
      render_texture(x, _y, w, _h);
      Blobs::render_debug(blobs.get(), x, _y, w, _h);
      _y += _h;

      if (send_pix != nullptr)
      {
        _y += LH;
        text("pix to send", x, _y);
        _y += LH/2;
        load_render(*send_pix, send_tex, x, _y, w, _h);
        _y += _h;
      }
    };

    void render_cam(float x, float y, float w, float h)
    {
      load_render(cam_pix, cam_tex, x, y, w, h);
      chilitags.render(x, y, w, h);
    };

    void render_texture(float x, float y, float w, float h)
    {
      if (proj_tex.isAllocated())
        proj_tex.draw(x, y, w, h);
    };

    void print_connections(float x, float& y)
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

    void print_metadata(float x, float& y)
    {
      float lines = 1;

      stringstream status;
      status << "metadata= \n";

      if (proj_pix.isAllocated())
      {
        status << " pixels proj:" 
            << " dim " 
              << proj_pix.getWidth() << "," 
              << proj_pix.getHeight()
            << " chan " << proj_pix.getNumChannels()
          << "\n";
        lines++;
      }

      if (proj_pix_resized.isAllocated())
      {
        status << " pixels proj resized:" 
            << " dim " 
              << proj_pix_resized.getWidth() << "," 
              << proj_pix_resized.getHeight()
            << " chan " << proj_pix_resized.getNumChannels()
          << "\n";
        lines++;
      }

      status << " calib:" 
          << " enabled " << _calib_enabled
        << "\n"
        << " juegos:"
          << " active " << juegos.active();
      lines++;

      ofDrawBitmapStringHighlight(status.str(), x, y);
      y += LH*lines;
    };

    void print_bloques(float x, float& y)
    {
      text("bloques", x, y);
      y += LH;

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
        text(status.str(), x, y);
        y += LH;
      }
    }; 

    void print_blobs(float x, float& y)
    {
      string status = !blobs_server.connected() ? "not connected" : ofToString(blobs.get().size()); 
      text("blobs: "+status, x, y);
      y += LH;
    };

    void dispose()
    {
      cam.dispose();
      calib.dispose();
      seg.dispose();
      blobs.dispose();
      cam_pix.clear();
      cam_tex.clear();
      proj_pix.clear();
      proj_pix_resized.clear();
      seg_pix_src.clear();
      seg_pix_dst.clear();
      chili_pix.clear();
      proj_tex.clear();
      proj_tags.clear();
      proj_bloques.clear();
      msg_server.dispose();
      bin_server.dispose();
      blobs_server.dispose();
      if (send_pix != nullptr)
      {
        (*send_pix).clear();
        send_tex.clear();
      }
    };

    ofTexture& texture()
    {
      return proj_tex;
    };

    //map<int, Bloque>& bloques()
    //{
      //return proj_bloques;
    //};

    bool calib_enabled()
    {
      return _calib_enabled;
    };

    float line_height()
    {
      return LH;
    };

  private:

    float proj_w, proj_h;
    float resize_seg, resize_send;
    float _calib_enabled;
    bool cam_updated;
    float LH;

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
    ofPixels seg_pix_src, seg_pix_dst;
    ofPixels proj_pix, proj_pix_resized;
    ofPixels* send_pix;
    ofTexture send_tex, cam_tex, proj_tex;

    vector<ChiliTag> proj_tags;
    map<int, Bloque> proj_bloques; 
 

    void segmentation_src(ofPixels& cam_pix, ofPixels& src)
    {
      float xscale, yscale;

      if (resize_seg > 0)
      {
        xscale = resize_seg;
        yscale = resize_seg;
      }
      else
      {
        xscale = ((float)proj_w) / cam_pix.getWidth();
        yscale = ((float)proj_h) / cam_pix.getHeight();
      }

      ofxMicromundos::resize(cam_pix, src, xscale, yscale);
    };

    void segmentation_dst(Segmentation& seg, ofPixels& dst)
    {
      //if seg_pix_src was down-scaled 
      //then up-scale it back to projection size 
      //for transformation
      if (resize_seg > 0)
      {
        ofPixels& _seg_pix = seg.pixels();
        float xscale = ((float)proj_w) / _seg_pix.getWidth();
        float yscale = ((float)proj_h) / _seg_pix.getHeight();
        ofxMicromundos::resize(_seg_pix, dst, xscale, yscale);
      }
      else
      {
        dst = seg.pixels();
      }
    };

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

    void print_ws_connection(
        ofxLibwebsockets::Server& server, 
        bool connected, 
        string name, 
        float x, float& y)
    {
      if (!connected)
      {
        ofDrawBitmapStringHighlight("ws server "+name+" not connected", x, y, ofColor::red, ofColor::black);
        y += LH;
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

        y += LH;
        ofDrawBitmapString(info, x, y);
        y += LH;
      }
    };

    void load_render(ofPixels& pix, ofTexture& tex, float x, float y, float w, float h)
    {
      if (pix.isAllocated())
        tex.loadData(pix);
      if (tex.isAllocated())
        tex.draw(x, y, w, h);
    };

    void text(string text, int x, int y)
    {
      ofDrawBitmapStringHighlight(text, x, y, ofColor::yellow, ofColor::black);
    };
};

