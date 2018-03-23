#pragma once

#include "ofxMicromundos/Bloque.h"

class Juegos
{
  public:

    Juegos() {};
    ~Juegos() {};

    void init(cv::FileNode juegos_config)
    {
      this->juegos_config = juegos_config;
      cur = "";
    };

    void update(map<int, Bloque> bloques)
    {
      for (cv::FileNodeIterator it = juegos_config.begin(); it != juegos_config.end(); it++)
      {
        string name = (*it).name();
        int id = (int)juegos_config[name]["tag_id"];
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
    };

  private:

    cv::FileNode juegos_config;
    string cur;
};

