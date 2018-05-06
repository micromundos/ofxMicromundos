#pragma once

#include "ofxSyphon.h"

class SyphonSender
{
  public:

    SyphonSender() {};
    ~SyphonSender() 
    {
      stop();
    };

    SyphonSender& init(string name)
    {
      this->name = name;
      server = nullptr;
      return *this;
    };

    void start()
    {
      server = new ofxSyphonServer();
      server->setName(name);
    };

    void stop()
    {
      if (server == nullptr)
        return;
      delete server;
      server = nullptr;
    };

    void publishScreen()
    {
      if (running())
        server->publishScreen();
    };

    void publishTexture(ofTexture* tex)
    {
      if (running())
        server->publishTexture(tex);
    };

    bool running()
    {
      return server != nullptr;
    };

  private:

    ofxSyphonServer* server;
    string name;
};

