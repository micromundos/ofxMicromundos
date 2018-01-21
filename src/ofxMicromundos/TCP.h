#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxMicromundos/Bloque.h"

//TODO tcp server: split in 2 servers (message & binary), throttle binary 

class TCP
{
  public:

    TCP() {};
    ~TCP() 
    {
      dispose();
    };

    void init(int port)
    {
      tcp.setup(port);
      //tcp.setMessageDelimiter(",");
    };

    void dispose()
    {
      tcp.disconnectAllClients();
      tcp.close();
    };

    bool send(ofPixels& pix, map<int, Bloque>& bloques)
    {
      if (!tcp.isConnected())
        return false; 

      string msg = "";

      //pixels:size=307200#dim=640,480#chan=1_bloques:id=0#loc=0,0#dir=0,0#ang=0;id=1#loc=1,1#dir=1,1#ang=1

      msg += "pixels:";
      msg += "size=" + ofToString(pix.size()) + "#"
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

      bool data_sent = tcp.sendToAll(msg);

      char* pix_data = reinterpret_cast<char*>(pix.getData()); 
      bool pix_sent = tcp.sendRawBytesToAll(pix_data, pix.size());

      return data_sent && pix_sent;
    };

    void render_info(float x, float y)
    {
      if (!tcp.isConnected())
      {
        ofDrawBitmapStringHighlight("tcp server not connected", x, y, ofColor::red, ofColor::black);
        return;
      }

      ofDrawBitmapStringHighlight("tcp server port: "+ofToString(tcp.getPort()), x, y, ofColor::green, ofColor::black);

      for (unsigned int i = 0; i < (unsigned int)tcp.getLastID(); i++)
      {
        if (!tcp.isClientConnected(i))
          continue;

        y += 24 + 20*i;

        string port = ofToString( tcp.getClientPort(i) );
        string ip = tcp.getClientIP(i);
        string info = "client "+ofToString(i)+" from "+ip+" on port: "+port;
        ofColor color = ofColor(255-i*30, 255-i*20, 100+i*40);

        ofDrawBitmapStringHighlight(info, x, y, color, ofColor::black);
      }
    };

  private:

    ofxTCPServer tcp;
};

