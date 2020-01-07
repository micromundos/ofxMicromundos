#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxCv.h"

#ifdef TARGET_OSX
  #define micromundos_USE_SYPHON
#endif

namespace ofxMicromundos { 

  /*
   * simple tweak to ofUtils.cpp#defaultDataPath
   * to get proper custom data paths
   * ("data" replaced by path arg)
   * usage: 
   * void ofApp::setup() {
   *   ofSetDataPathRoot(ofxMicromundos::getDataPath("my/data/path/without/leading/nor/trailing/slashes"));
   * }
   */
  string getDataPath(string path);
  void setDataPathRoot();

  ofxJSONElement load_config(string filename);

  ofVec2f projector_position(ofxJSONElement& config);
  void projector(ofxJSONElement& config);

  vector<ofVec2f> calib_points(float proj_w, float proj_h);

  void resize(ofPixels& src, ofPixels& dst, float xscale, float yscale);
};
