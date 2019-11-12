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

  ofxJSON load_config(string filename)
  {
    ofxJSON json;
    bool loaded = json.open(filename);
    if (!loaded)
    {
      ofLogError() << "failed to load " << filename;
      ofExit();
    }
    return json;
  };

  ofVec2f projector_position(ofxJSON& config)
  {
    string pos = config["projector"]["position"].asString();
    float x0;
    if (pos == "right")
    {
      x0 = ofGetScreenWidth();
    }
    else if (pos == "left")
    {
      float w = config["projector"]["width"].asFloat();
      x0 = -w;
    }
    else if (pos == "center")
    {
      x0 = 0;
    }
    float x = config["projector"]["x"].asFloat();
    return ofVec2f(x0 + x, config["projector"]["y"].asFloat());
  }; 

  void projector(ofxJSON& config)
  {
    ofSetWindowShape(config["projector"]["width"].asInt(), config["projector"]["height"].asInt());
    ofVec2f proj = projector_position(config);
    ofSetWindowPosition(proj.x, proj.y);
    ofSetFullscreen(config["projector"]["fullscreen"].asString().compare("true") == 0);
  };

  vector<ofVec2f> calib_points(float proj_w, float proj_h)
  {
    //pts: [ 
      //[ 0.33, 0.33 ], [ 0.67, 0.33 ], 
      //[ 0.33, 0.67 ], [ 0.67, 0.67 ] 
    //]

    vector<ofVec2f> pts;

    pts.push_back(ofVec2f( proj_w * 0.33, proj_h * 0.33 ));
    pts.push_back(ofVec2f( proj_w * 0.67, proj_h * 0.33 ));
    //XXX clockwise
    pts.push_back(ofVec2f( proj_w * 0.67, proj_h * 0.67 ));
    pts.push_back(ofVec2f( proj_w * 0.33, proj_h * 0.67 ));

    return pts;
  };
};
