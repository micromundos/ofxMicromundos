#pragma once

namespace ofxMicromundos { 
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
