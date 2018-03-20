#pragma once

#include "ofxMicromundos/Bloque.h"

class Cartuchos
{
  public:

    Cartuchos() {};
    ~Cartuchos() {};

    void init(cv::FileStorage cartuchos_config)
    {
      this->cartuchos_config = cartuchos_config;
      cur = -1;
    };

    void update(map<int, Bloque> bloques)
    {
      cv::FileNode root = cartuchos_config.root();
      for (cv::FileNodeIterator it = root.begin(); it != root.end(); it++)
      {
        int id = (int)root[(*it).name()];
        if (bloques.find(id) != bloques.end())
        {
          cur = id;
          break;
        }
      }
    };

    bool active(string name)
    {
      return cur == (int)cartuchos_config[name];
    };

  private:

    cv::FileStorage cartuchos_config;
    int cur;
};

