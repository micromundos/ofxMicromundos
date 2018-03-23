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
      cur = "";
    };

    void update(map<int, Bloque> bloques)
    {
      cv::FileNode root = cartuchos_config.root();
      for (cv::FileNodeIterator it = root.begin(); it != root.end(); it++)
      {
        string name = (*it).name();
        int id = (int)root[name];
        if (bloques.find(id) != bloques.end())
        {
          cur = name;
          break;
        }
      }
    };

    string active()
    {
      return cur;
    };

    bool active(string name)
    {
      return cur == name;
      //return cur == (int)cartuchos_config[name];
    };

  private:

    cv::FileStorage cartuchos_config;
    string cur;
};

