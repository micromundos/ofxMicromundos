#include "ofxMicromundos/data_path.h"

namespace ofxMicromundos { 

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

