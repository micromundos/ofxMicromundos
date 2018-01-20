#include "ofxMicromundos/utils.h"

namespace ofxMicromundos { 

  string getDataPath(string path)
  {
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
  };

  void setDataPathRoot()
  {
    ofSetDataPathRoot(getDataPath("../../data"));
  };

  ofVec2f projector_position(cv::FileStorage config)
  {
    string pos = config["projector"]["position"];
    float x0;
    if (pos == "right")
    {
      x0 = ofGetScreenWidth();
    }
    else if (pos == "left")
    {
      float w = config["projector"]["width"];
      x0 = -w;
    }
    else if (pos == "center")
    {
      x0 = 0;
    }
    float x = config["projector"]["x"];
    return ofVec2f(x0 + x, config["projector"]["y"]);
  };

  cv::FileStorage load_config(string filename)
  {
    cv::FileStorage cfg( ofToDataPath(filename, false), cv::FileStorage::READ );
    if (!cfg.isOpened())
    {
      ofLogError() << "failed to load " << filename;
      ofExit();
    }
    return cfg;
  };

};
