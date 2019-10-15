#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"
#include "ofxMicromundos/Bloque.h"

class BinServer
{
  public:

    BinServer() {};
    ~BinServer() 
    {
      dispose();
    };

    void init(int port)
    {
      ofxLibwebsockets::ServerOptions opt = ofxLibwebsockets::defaultServerOptions();
      opt.port = port;
      _server.setup(opt); 
    };

    void dispose()
    {
      _server.exit();
    };

    bool send(ofPixels& out_pix, bool enabled)
    {
      if (!enabled || !connected())
        return false;
      _server.sendBinary(out_pix.getData(), out_pix.getTotalBytes());
      return true;
    };

    bool connected()
    {
      return _server.getConnections().size() > 0;
    };

    ofxLibwebsockets::Server& server()
    {
      return _server;
    };

  private:

    ofxLibwebsockets::Server _server;
};

