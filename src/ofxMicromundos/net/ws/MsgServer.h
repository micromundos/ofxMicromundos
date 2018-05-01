#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"
#include "ofxMicromundos/Bloque.h"

class MsgServer
{
  public:

    MsgServer() {};
    ~MsgServer() 
    {
      dispose();
    };

    void init(int port)
    {
      ofxLibwebsockets::ServerOptions cfg = ofxLibwebsockets::defaultServerOptions();
      cfg.port = port;
      _server.setup(cfg); 
    };

    void dispose()
    {
      _server.exit();
    };

    bool send( 
        ofPixels& out_pix,
        map<int, Bloque>& bloques, 
        bool message_enabled, 
        bool binary_enabled,
        bool syphon_enabled,
        bool calib_enabled, 
        string juego_active)
    {
      if (!message_enabled || !connected())
        return false;
      _server.send(serialize(out_pix, bloques, binary_enabled, syphon_enabled, calib_enabled, juego_active));
      return true;
    };

    //pixels:dim=640,480#chan=1
    //_net:bin=1#syphon=0
    //_calib:enabled=1
    //_juegos:active=pepe
    //_bloques:id=0#loc=0,0#dir=0,0#ang=0#r=0;id=1#loc=1,1#dir=1,1#ang=1#r=1
    string serialize(ofPixels& out_pix, map<int, Bloque>& bloques, bool binary_enabled, bool syphon_enabled, bool calib_enabled, string juego_active)
    {
      string msg = "";

      msg += "pixels:";
      msg += "dim=" 
          + ofToString(out_pix.getWidth()) + ","
          + ofToString(out_pix.getHeight()) + "#"
        + "chan=" 
          + ofToString(out_pix.getNumChannels());  

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

