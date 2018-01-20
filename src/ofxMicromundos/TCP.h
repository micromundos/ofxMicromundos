#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxMicromundos/Bloque.h"

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

    bool send_pixels(ofPixels& pix)
    {
      //if (!tcp.isConnected())
        return false;
      //return tcp.sendRawBytesToAll(const char * rawBytes, const int numBytes);
    };

    bool send_bloques(map<int, Bloque>& bloques)
    {
      if (!tcp.isConnected())
        return false;

      string msg = "";
      int i = 0;

      for (const auto& bloque : bloques)
      {
        const Bloque& b = bloque.second;
        string sep = i++ > 0 ? ";" : "";
        msg += sep + "id:" + ofToString(b.id) + "_" + "loc:" + ofToString(b.loc.x) + "," + ofToString(b.loc.y) + "_" + "dir:" + ofToString(b.dir.x) + "," + ofToString(b.dir.y) + "_" + "angle:" + ofToString(b.angle);
      }

      return tcp.sendToAll(msg);
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
        ofColor color = ofColor(255-i*30, 255-i*20, 100+i*40);
        int xPos = x;
        int yPos = y+24 + 20*i;
        string port = ofToString( tcp.getClientPort(i) );
        string ip = tcp.getClientIP(i);
        string info = "client "+ofToString(i)+" from "+ip+" on port: "+port;
        ofDrawBitmapStringHighlight(info, xPos, yPos, color, ofColor::black);
      }
    };

  private:

    ofxTCPServer tcp;
};

