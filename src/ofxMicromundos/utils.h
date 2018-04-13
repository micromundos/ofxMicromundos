#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

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
};
