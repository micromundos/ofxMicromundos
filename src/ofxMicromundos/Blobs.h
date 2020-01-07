#pragma once

#include "ofxOpenCv.h"
#include "ofxCv.h"

class Blobs
{ 

  public:

    Blobs() {};
    ~Blobs() 
    {
      dispose();
    };

    void dispose()
    {
      blobs.clear();
    };

    void init()
    {
      resampled_len = 100.0f;
      interpolation_coef = 0.3f;
      blobs_threshold = 15;
      interpolate_ = true;

      contours.setMinAreaRadius( 10 );
      contours.setMaxAreaRadius( 1000 );
      //contours.setAutoThreshold(true); // invert works with autothreshold only
      //contours.setInvert(true); // find black instead of white

      ofxCv::RectTracker& tracker = contours.getTracker();
      tracker.setPersistence( 20 );
      tracker.setMaximumDistance( 100 );
    };

    void update(ofPixels& pix)
    {
      if (!pix.isAllocated())
        return;

      int w = pix.getWidth();
      int h = pix.getHeight();

      contours.setThreshold( blobs_threshold );
      contours.findContours( pix );

      const vector<ofPolyline>& _blobs = contours.getPolylines();
      ofxCv::RectTracker& tracker = contours.getTracker();
      int len = contours.size();

      blobs.clear();

      for (unsigned int i = 0; i < len; i++)
      {
        unsigned int label = contours.getLabel(i);

        if ( interpolate_ && tracker.existsPrevious(label) ) 
        {
          ofPolyline& prev = _blobs_prev[label];

          ofPolyline interpolated = interpolate(prev, _blobs[i]);
          blobs.push_back( interpolated );

          prev.clear();
          prev.addVertices( interpolated.getVertices() );
        }

        else
        {
          ofPolyline resampled = _blobs[i].getResampledByCount(resampled_len);
          blobs.push_back( resampled );

          if ( interpolate_ )
          {
            ofPolyline& prev = _blobs_prev[label];
            prev.clear();
            prev.addVertices(resampled.getVertices());
          }
        }

        normalize_blob(blobs[i], w, h);
      }
    }; 

    static void render_debug(vector<ofPolyline>& _blobs, float x, float y, float w, float h)
    {
      ofPushStyle();
      ofSetColor(ofColor::orange);
      ofSetLineWidth(2);
      ofPushMatrix();
      ofTranslate(x, y);
      ofScale(w, h); 

      for (const auto& blob : _blobs)
        blob.draw();

      ofPopMatrix();
      ofPopStyle();
    }; 

    vector<ofPolyline>& get()
    {
      return blobs;
    };

  private:

    //ofPixels pix;

    ofxCv::ContourFinder contours; 
    map<unsigned int, ofPolyline> _blobs_prev;
    vector<ofPolyline> blobs;

    float resampled_len;
    float interpolation_coef;
    float blobs_threshold;
    bool interpolate_;

    void normalize_blob(ofPolyline& blob, int w, int h)
    {
      int plen = blob.size();
      for (int j = 0; j < plen; j++)
      {
        blob[j].x /= w;
        blob[j].y /= h;
      }
    };

    ofPolyline interpolate(
        const ofPolyline& src, 
        const ofPolyline& dst)
    {
      ofPolyline dst1 = dst.getResampledByCount(resampled_len);
      dst1.close();

      //already resampled
      ofPolyline src1( src.getVertices() );
      //ofPolyline src1 = src.getResampledByCount(resampled_len);
      src1.close();

      int len = min( dst1.size(), src1.size() );
      dst1.resize( len );
      dst1.close();
      src1.resize( len );
      src1.close();

      for (int i = 0; i < len; i++)
        src1[i].interpolate(dst1[i], interpolation_coef);

      return src1;
    }; 
};

