#pragma once

#include "ofxChilitags.h"
#include "ofxMicromundos/RGB.h"
#include "ofxMicromundos/WebSockets.h"
#include "ofxMicromundos/Calib.h"
#include "ofxMicromundos/Segmentation.h"
#include "ofxMicromundos/Bloque.h"

//TODO Backend with ofThread
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
        string calib_file, 
        int calib_tag_id,
        float calib_tags_size,
        int port_bin = 0,
        int port_msg = 0)
    {
      this->proj_w = proj_w;
      this->proj_h = proj_h;

      calib_enabled = false;
      _updated = false;

      calib.init(
          proj_w, proj_h, 
          calib_file, 
          calib_tag_id,
          calib_tags_size);

      cam.init(cam_w, cam_h, cam_device_id);
      chilitags.init(); 
      seg.init();

      if (port_bin != 0 && port_msg != 0)
        server.init(port_bin, port_msg);
    };

    bool update()
    {
      _updated = cam.update();

      if (!_updated)
        return false;

      ofPixels &cam_pix = cam.pixels();

      chilitags.update(cam_pix);
      vector<ChiliTag>& tags = chilitags.tags();

      seg.update(cam_pix, tags); 

      calib_enabled = calib.enabled(tags);
      if (calib_enabled)
        calib.find(tags, proj_w, proj_h);

      calib.transform(seg.pixels(), proj_pix, proj_w, proj_h);
      proj_tex.loadData(proj_pix);

      calib.transform(tags, proj_tags, proj_w, proj_h);
      tags_to_bloques(proj_tags, proj_bloques);

      proj_pix_out.setFromPixels(proj_pix.getData(), proj_pix.getWidth(), proj_pix.getHeight(), proj_pix.getNumChannels());

      return true;
    };

    bool send()
    {
      if (!_updated) return false;
      return server.send(proj_pix_out, proj_bloques);
    };

    bool render_calib(float w, float h)
    {
      if (calib_enabled)
        calib.render();
      return calib_enabled;
    };

    void render_projected_pixels(float w, float h)
    { 
      if (proj_tex.isAllocated())
        proj_tex.draw(0, 0, w, h);
    };

    void render_projected_tags()
    {
      ofPushStyle();
      ofSetColor(ofColor::magenta);
      ofSetLineWidth(4);
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
      float _w = w/2;
      //left
      cam.render(x, y, _w, h);
      chilitags.render(x, y, _w, h);
      //right
      seg.render(x + _w, y, _w, h);
    };

    void render_server_info(float x, float y)
    { 
      server.render_info(x, y);
    }; 

    void dispose()
    {
      cam.dispose();
      calib.dispose();
      seg.dispose();
      proj_pix.clear();
      proj_pix_out.clear();
      proj_tex.clear();
      proj_tags.clear();
      proj_bloques.clear();
      server.dispose();
    };

    ofPixels& projected_pixels()
    {
      return proj_pix_out;
    };

    ofTexture& projected_texture()
    {
      return proj_tex;
    };

    map<int, Bloque>& projected_bloques()
    {
      return proj_bloques;
    };


  private:

    float calib_enabled;
    float proj_w, proj_h;
    bool _updated;

    RGB cam;
    WebSockets server;
    Calib calib;
    Segmentation seg;
    ofxChilitags chilitags;

    ofPixels proj_pix;
    ofPixels proj_pix_out;
    ofTexture proj_tex;

    vector<ChiliTag> proj_tags;
    map<int, Bloque> proj_bloques; 


    void tags_to_bloques(vector<ChiliTag>& tags, map<int, Bloque>& bloques)
    {
      map<int,bool> _tags;

      for (int i = 0; i < tags.size(); i++)
      {
        int id = tags[i].id;
        _tags[id] = true;
        if (bloques.find(id) == bloques.end())
          make_bloque(tags[i], bloques);
        else 
          update_bloque(tags[i], bloques[id]);
      }

      vector<int> remove;
      for (const auto& bloque : bloques)
      {
        int id = bloque.first;
        if (_tags.find(id) == _tags.end())
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
      b.corners = t.corners_n;
      b.dir = t.dir;
      b.angle = t.angle;
    };

    void interpolate_bloque(ChiliTag& t, Bloque& b)
    {
      b.loc_i += (t.center_n - b.loc_i) * 0.2;
      b.dir_i += (t.dir - b.dir_i) * 0.2;
      b.angle_i = ofLerpRadians(b.angle_i, t.angle, 0.05);
    };

};
