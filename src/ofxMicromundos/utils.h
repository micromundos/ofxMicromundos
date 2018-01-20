#pragma once

#include "ofMain.h"
#include "opencv2/opencv.hpp"

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

  ofVec2f projector_position(cv::FileStorage config);
  cv::FileStorage load_config(string filename);
};
