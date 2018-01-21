#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"
#include "ofxMicromundos/Bloque.h"

class WebSockets
{
  public:

    WebSockets() {};
    ~WebSockets() 
    {
      dispose();
    };

    void init(int port)
    {
      ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
      options.port = port;
      connected = server.setup(options);
    };

    void dispose()
    {
      server.exit();
    };

    bool send(ofPixels& pix, map<int, Bloque>& bloques)
    {
      if (!connected)
        return false; 

      string msg = "";

      //pixels:size=307200#dim=640,480#chan=1_bloques:id=0#loc=0,0#dir=0,0#ang=0;id=1#loc=1,1#dir=1,1#ang=1

      msg += "pixels:";
      //msg += "size=" + ofToString(pix.size()) + "#"
      msg += "size=" + ofToString(pix.getTotalBytes()) + "#"
        + "dim=" 
          + ofToString(pix.getWidth()) + ","
          + ofToString(pix.getHeight()) + "#"
        + "chan=" 
          + ofToString(pix.getNumChannels());

      msg += "_bloques:";
      int i = 0;
      for (const auto& bloque : bloques)
      {
        const Bloque& b = bloque.second;
        string sep = i++ > 0 ? ";" : "";
        msg += sep 
          + "id=" + ofToString(b.id) + "#" 
          + "loc=" 
            + ofToString(b.loc.x) + "," 
            + ofToString(b.loc.y) + "#" 
          + "dir=" 
            + ofToString(b.dir.x) + "," 
            + ofToString(b.dir.y) + "#" 
          + "ang=" + ofToString(b.angle);
      }

      //TODO send message + pixels
      //server.send(msg);
      //server.sendBinary(pix.getData(), pix.size());
      server.sendBinary(pix.getData(), pix.getTotalBytes());

      return true;
    };

    void render_info(float x, float y)
    {
      if (!connected)
      {
        ofDrawBitmapStringHighlight("websockets server not connected", x, y, ofColor::red, ofColor::black);
        return;
      }

      ofDrawBitmapStringHighlight("websockets server port: "+ofToString(server.getPort()), x, y, ofColor::green, ofColor::black);

      vector<ofxLibwebsockets::Connection*> conns = server.getConnections();
      for (int i = 0; i < conns.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns[i];

        y += 24 + 20*i;

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from "+ip;
        ofColor color = ofColor(255-i*30, 255-i*20, 100+i*40);

        ofDrawBitmapStringHighlight(info, x, y, color, ofColor::black);
      }
    };

  private:

    ofxLibwebsockets::Server server;
    bool connected;
};

