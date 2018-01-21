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

    void init(int port_bin, int port_msg)
    {
      ofxLibwebsockets::ServerOptions bin = ofxLibwebsockets::defaultServerOptions();
      bin.port = port_bin;

      ofxLibwebsockets::ServerOptions msg = ofxLibwebsockets::defaultServerOptions();
      msg.port = port_msg;

      connected = server_bin.setup(bin) && server_msg.setup(msg);
    };

    void dispose()
    {
      server_bin.exit();
      server_msg.exit();
    };

    bool send(ofPixels& pix, map<int, Bloque>& bloques)
    {
      if (!connected)
        return false; 
      server_msg.send(message(pix, bloques));
      server_bin.sendBinary(pix.getData(), pix.getTotalBytes());
      return true;
    };

    //pixels:size=307200#dim=640,480#chan=1_bloques:id=0#loc=0,0#dir=0,0#ang=0;id=1#loc=1,1#dir=1,1#ang=1
    string message(ofPixels& pix, map<int, Bloque>& bloques)
    {
      string msg = "";

      msg += "pixels:";
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

      return msg;
    };

    void render_info(float x, float y)
    {
      if (!connected)
      {
        ofDrawBitmapStringHighlight("websockets server not connected", x, y, ofColor::red, ofColor::black);
        return;
      }

      float lh = 24; 

      ofDrawBitmapStringHighlight("websockets server bin port: "+ofToString(server_bin.getPort()), x, y, ofColor::green, ofColor::black);
      y += lh; 

      vector<ofxLibwebsockets::Connection*> conns_bin = server_bin.getConnections();
      for (int i = 0; i < conns_bin.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns_bin[i];

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from "+ip;
        ofColor color = ofColor(255-i*30, 255-i*20, 100+i*40);

        ofDrawBitmapStringHighlight(info, x, y, color, ofColor::black);
        y += lh;
      }

      y += lh; 

      ofDrawBitmapStringHighlight("websockets server msg port: "+ofToString(server_msg.getPort()), x, y, ofColor::green, ofColor::black);
      y += lh;

      vector<ofxLibwebsockets::Connection*> conns_msg = server_msg.getConnections();
      for (int i = 0; i < conns_msg.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns_msg[i];

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from "+ip;
        ofColor color = ofColor(255-i*30, 255-i*20, 100+i*40);

        ofDrawBitmapStringHighlight(info, x, y, color, ofColor::black);
        y += lh;
      }
    };

  private:

    ofxLibwebsockets::Server server_msg;
    ofxLibwebsockets::Server server_bin;
    bool connected;
};

