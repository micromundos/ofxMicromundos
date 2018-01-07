#pragma once

#include "ofxChilitags.h"
#include "ofxMicromundos/RGB.h"
#include "ofxMicromundos/Calib.h"
#include "ofxMicromundos/Segmentation.h"

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
      calib.transform(tags, proj_tags, w, h);

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

    void render_monitor(float w, float h)
    {
      float hw = w/2;
      float hh = h/2;

      //top left
      rgb.render(0, 0, hw, hh);
      chilitags.render(0, 0, hw, hh);

      //top right
      seg.render(hw, 0, hw, hh);
    };

    void dispose()
    {
      rgb.dispose();
      calib.dispose();
      seg.dispose();
      proj_pix.clear();
      proj_tex.clear();
      proj_tags.clear();
    };

    ofPixels& projected_pixels()
    {
      return proj_pix;
    };

    ofTexture& projected_texture()
    {
      return proj_tex;
    };

    vector<ChiliTag>& projected_tags()
    {
      return proj_tags;
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

    void render_proj_pix(float w, float h)
    {
      if (proj_pix.isAllocated())
        proj_tex.loadData(proj_pix);
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

};
