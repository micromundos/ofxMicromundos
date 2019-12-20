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
      cur = default_juego();
    };

    void update(map<int, Bloque> bloques)
    {
      vector<string> keys = juegos_config.getMemberNames();
      for (const auto& name : keys)
      {
        int id = juegos_config[name]["tag_id"].asInt();
        if (bloques.find(id) != bloques.end())
        {
          cur = name;
          break;
        }
      }
    };

    string default_juego()
    {
      return juegos_config.getMemberNames()[0];
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

