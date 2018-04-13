#pragma once

#include "ofxMicromundos/Bloque.h"

class Juegos
{
  public:

    Juegos() {};
    ~Juegos() {};

    void init(const Json::Value& juegos_config)
    {
      this->juegos_config = juegos_config;
      cur = "";
    };

    void update(map<int, Bloque> bloques)
    {
      vector<string> keys = juegos_config.getMemberNames();
      for (auto& name : keys)
      {
        int id = juegos_config[name]["tag_id"].asInt();
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
      return cur.compare(name) == 0;
    };

  private:

    Json::Value juegos_config;
    string cur;
};

