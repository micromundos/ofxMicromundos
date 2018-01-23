#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"
#include "ofxMicromundos/Bloque.h"

//serialization
//pixels:dim=640,480#chan=1_bloques:id=0#loc=0,0#dir=0,0#ang=0;id=1#loc=1,1#dir=1,1#ang=1#radio=1

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
      out_pix.clear();
      server_bin.exit();
      server_msg.exit();
    };

    bool send(ofPixels& pix, map<int, Bloque>& bloques, float resize)
    {
      if (!connected)
        return false; 

      ofPixels* opix;
      if (resize != 1.0)
      {
        ofxCv::resize(pix, out_pix, resize, resize);
        opix = &out_pix;
      }
      else
        opix = &pix;

      server_msg.send(serialize(*opix, bloques));
      server_bin.sendBinary(opix->getData(), opix->getTotalBytes());

      return true;
    };

    string serialize(ofPixels& pix, map<int, Bloque>& bloques)
    {
      string msg = "";

      msg += "pixels:";
      msg += "dim=" 
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
          + "ang=" + ofToString(b.angle) + "#"
          + "r=" + ofToString(b.radio);

        //for (int j = 0; j < b.corners.size(); j++)
        //{
          //ofVec2f& corner = b.corners[j];
          //string tail = j == b.corners.size()-1 ? "" : "#";
          //msg += "corner@"+j+"="
            //+ ofToString(corner.x) + "," 
            //+ ofToString(corner.y) + tail;
        //}
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
        string info = "client "+name+" from ip "+ip;
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
    ofPixels out_pix;
    bool connected;
};

