#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"
#include "ofxMicromundos/Bloque.h"

//TODO WebSockets
//move to 
  //net/ws/MsgServer
  //net/ws/BinServer
  //net/ws/BlobsServer

class WebSockets
{
  public:

    WebSockets() {};
    ~WebSockets() 
    {
      dispose();
    };

    void init(int port_bin, int port_msg, int port_blobs)
    {
      ofxLibwebsockets::ServerOptions bin = ofxLibwebsockets::defaultServerOptions();
      bin.port = port_bin;

      ofxLibwebsockets::ServerOptions msg = ofxLibwebsockets::defaultServerOptions();
      msg.port = port_msg;

      ofxLibwebsockets::ServerOptions blobs = ofxLibwebsockets::defaultServerOptions();
      blobs.port = port_blobs;

      connected = server_bin.setup(bin) && server_msg.setup(msg) && server_blobs.setup(blobs); 
    };

    void dispose()
    {
      out_pix.clear();
      server_bin.exit();
      server_msg.exit();
      server_blobs.exit();
    };

    bool send( 
        ofPixels& pix,
        map<int, Bloque>& bloques, 
        vector<ofPolyline>& blobs,
        bool message_enabled, 
        bool binary_enabled,
        bool syphon_enabled,
        bool blobs_enabled,
        bool calib_enabled, 
        string juego_active,
        float resize)
    {
      if (!message_enabled && !binary_enabled && !blobs_enabled)
        return false;

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

      if (message_enabled)
        server_msg.send(serialize(*opix, bloques, binary_enabled, syphon_enabled, calib_enabled, juego_active));

      if (binary_enabled)
        server_bin.sendBinary(opix->getData(), opix->getTotalBytes());

      if (blobs_enabled)
        server_blobs.send(serialize_blobs(blobs));

      return true;
    };

    //pixels:dim=640,480#chan=1
    //_net:bin=1#syphon=0
    //_calib:enabled=1
    //_juegos:active=pepe
    //_bloques:id=0#loc=0,0#dir=0,0#ang=0#r=0;id=1#loc=1,1#dir=1,1#ang=1#r=1
    string serialize(ofPixels& pix, map<int, Bloque>& bloques, bool binary_enabled, bool syphon_enabled, bool calib_enabled, string juego_active)
    {
      string msg = "";

      msg += "pixels:";
      msg += "dim=" 
          + ofToString(pix.getWidth()) + ","
          + ofToString(pix.getHeight()) + "#"
        + "chan=" 
          + ofToString(pix.getNumChannels());  

      msg += "_net:";
      msg += "bin=" + ofToString(binary_enabled) + "#"
        + "syphon=" + ofToString(syphon_enabled); 

      msg += "_calib:enabled=" 
        + ofToString(calib_enabled);

      msg += "_juegos:active=" 
        + juego_active;

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

    //TODO serialize_blobs + deserialize_blobs
    //blobs:0,0;1,1;2,2#0,0;1,1;2,2
    string serialize_blobs(vector<ofPolyline>& blobs)
    {
      string msg = "";
      msg += "blobs:";

      int i = 0;
      for (const auto& blob : blobs)
      {
        string blob_sep = i++ > 0 ? "#" : "";
        msg += blob_sep;

        const vector<ofPoint>& pts = blob.getVertices();

        int j = 0;
        for (const auto& pt : pts)
        {
          string pt_sep = j++ > 0 ? ";" : "";

          msg += pt_sep 
              + ofToString(pt.x) + "," 
              + ofToString(pt.y);
        }
      }

      return msg;
    };

    void print_connection(float x, float y)
    {
      if (!connected)
      {
        ofDrawBitmapStringHighlight("websockets server not connected", x, y, ofColor::red, ofColor::black);
        return;
      }

      float lh = 24; 

      ofDrawBitmapStringHighlight("websockets server bin port: "+ofToString(server_bin.getPort()), x, y, ofColor::green, ofColor::black);

      vector<ofxLibwebsockets::Connection*> conns_bin = server_bin.getConnections();
      for (int i = 0; i < conns_bin.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns_bin[i];

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from ip "+ip;

        y += lh;
        ofDrawBitmapString(info, x, y);
      }

      y += lh; 


      ofDrawBitmapStringHighlight("websockets server msg port: "+ofToString(server_msg.getPort()), x, y, ofColor::green, ofColor::black);

      vector<ofxLibwebsockets::Connection*> conns_msg = server_msg.getConnections();
      for (int i = 0; i < conns_msg.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns_msg[i];

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from ip "+ip;

        y += lh;
        ofDrawBitmapString(info, x, y);
      }


      ofDrawBitmapStringHighlight("websockets server blobs port: "+ofToString(server_blobs.getPort()), x, y, ofColor::green, ofColor::black);

      vector<ofxLibwebsockets::Connection*> conns_blobs = server_blobs.getConnections();
      for (int i = 0; i < conns_blobs.size(); i++)
      {
        ofxLibwebsockets::Connection* conn = conns_blobs[i];

        string name = conn->getClientName();
        string ip = conn->getClientIP();
        string info = "client "+name+" from ip "+ip;

        y += lh;
        ofDrawBitmapString(info, x, y);
      }
    };

    bool bin_connected()
    {
      return server_bin.getConnections().size() > 0;
    };

    bool msg_connected()
    {
      return server_msg.getConnections().size() > 0;
    };

    bool blobs_connected()
    {
      return server_blobs.getConnections().size() > 0;
    };

  private:

    ofxLibwebsockets::Server server_msg;
    ofxLibwebsockets::Server server_bin;
    ofxLibwebsockets::Server server_blobs;
    ofPixels out_pix;
    bool connected;
};

