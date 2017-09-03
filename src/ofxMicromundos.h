#pragma once

#include "ofMain.h"

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
  string getDataPath(string path){
#if defined TARGET_OSX
    try{
      return std::filesystem::canonical(ofFilePath::join(ofFilePath::getCurrentExeDir(),  "../../../"+path+"/")).string();
    }catch(...){
      return ofFilePath::join(ofFilePath::getCurrentExeDir(),  "../../../"+path+"/");
    }
#elif defined TARGET_ANDROID
    return string("sdcard/");
#else
    try{
      return std::filesystem::canonical(ofFilePath::join(ofFilePath::getCurrentExeDir(),  path+"/")).string();
    }catch(...){
      return ofFilePath::join(ofFilePath::getCurrentExeDir(),  path+"/");
    }
#endif
  }

  void setDataPathRoot()
  {
    ofSetDataPathRoot(getDataPath("../../data"));
  }

}

