#pragma once

#include "ofxChilitags.h"
#include "ofxMicromundos/RGB.h"
#include "ofxMicromundos/Calib.h"
#include "ofxMicromundos/Segmentation.h"
#include "ofxMicromundos/Bloque.h"

class Backend
{
  public:

    Backend() {};
    ~Backend() 
    {
      dispose();
    };

    void init(float w, float h)
    {
      rgb.init();
      chilitags.init(); 
      calib.init(w, h);
      seg.init();
    };

    bool update(float w, float h)
    {
      if (!rgb.update())
        return false;

      ofPixels& rgb_pix = rgb.pixels();
      chilitags.update(rgb_pix);
      vector<ChiliTag>& tags = chilitags.tags();
      seg.update(rgb_pix, tags); 

      calib_enabled = calib.enabled(tags);
      if (calib_enabled)
        calib.find(tags, w, h);

      //copy before transform
      proj_pix = seg.pixels();
      calib.transform(proj_pix, w, h);
      proj_tex.loadData(proj_pix);

      calib.transform(tags, proj_tags, w, h);
      tags_to_bloques(proj_tags, proj_bloques);

      return true;
    };

    void render_calib(float w, float h)
    {
      if (calib_enabled)
        calib.render();
    };

    void render_projected(float w, float h)
    {
      render_proj_pix(w, h);
      render_proj_tags();
    };

    void render_monitor(float x, float y, float w, float h)
    {
      //left
      rgb.render(x, y, w, h);
      chilitags.render(x, y, w, h);
      //right
      seg.render(x + w, y, w, h);
    };

    void dispose()
    {
      rgb.dispose();
      calib.dispose();
      seg.dispose();
      proj_pix.clear();
      proj_tex.clear();
      proj_tags.clear();
      proj_bloques.clear();
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


  private:

    float calib_enabled;

    RGB rgb;
    Calib calib;
    Segmentation seg;
    ofxChilitags chilitags;

    ofPixels proj_pix;
    ofTexture proj_tex;

    vector<ChiliTag> proj_tags;
    map<int, Bloque> proj_bloques;
 

    void render_proj_pix(float w, float h)
    { 
      if (proj_tex.isAllocated())
        proj_tex.draw(0, 0, w, h);
    };

    void render_proj_tags()
    {
      ofPushStyle();
      ofSetColor(ofColor::magenta);
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


    void tags_to_bloques(vector<ChiliTag>& tags, map<int, Bloque>& bloques)
    {
      for (int i = 0; i < tags.size(); i++)
      {
        int id = tags[i].id;
        if (bloques.find(id) != bloques.end())
          update_bloque(tags[i], bloques[id]);
        else 
          bloques[id] = make_bloque(tags[i]);
      }
    };

    Bloque make_bloque(ChiliTag& t)
    {
      Bloque b;
      set_bloque(t, b);
      b.loc_i = b.loc;
      b.dir_i = b.dir;
      b.angle_i = b.angle;
      return b; 
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
    };

    void interpolate_bloque(ChiliTag& t, Bloque& b)
    {
      b.loc_i += (t.center_n - b.loc_i) * 0.2;
      b.dir_i += (t.dir - b.dir_i) * 0.2;
      b.angle_i = ofLerpRadians(b.angle_i, t.angle, 0.05);
    };

};
