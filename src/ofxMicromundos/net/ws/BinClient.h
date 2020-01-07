#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"

class BinClient
{
  public:

    BinClient() {};
    ~BinClient() {};

    void init(string host, int port)
    {
      if (port <= 0)
        return;
      ofxLibwebsockets::ClientOptions opt = ofxLibwebsockets::defaultClientOptions();
      opt.host = host;
      opt.port = port;
      opt.reconnect = true;
      opt.reconnectInterval = 5000;
      client.connect(opt);
      //client.connect(host, port);
      client.addListener(this);

      received = false;
      locked = false;
    };

    void dispose()
    {
      client.exit();
      pix.clear();
      tex.clear();
    };

    bool update(int pix_w, int pix_h, int pix_chan)
    {
      if ( !received )
        return false;
      deserialize(data, pix_w, pix_h, pix_chan);
      received = false;
      locked = false;
      return true;
    };

    void render(float x, float y, float w, float h)
    {
      if (tex.isAllocated())
        tex.draw(x, y, w, h);
    };

    ofPixels& pixels()
    {
      return pix;
    };

    ofTexture& texture()
    {
      return tex;
    };

    void print_connection(float x, float& y, float LH)
    {
      ofxLibwebsockets::Connection* conn = client.getConnection();
      if (conn == nullptr)
      {
        string info = "bin not connected"; 
        ofDrawBitmapStringHighlight(info, x, y);
        y += LH;
        return;
      }

      string name = conn->getClientName();
      string ip = conn->getClientIP();
      string info = "bin connected: name="+name + " / ip=" + ip; 
      ofDrawBitmapStringHighlight(info, x, y);
      y += LH;
    };

    bool connected()
    {
      return client.isConnected();
    };

    void onMessage( ofxLibwebsockets::Event& args )
    {
      if ( !args.isBinary || locked )
        return; 

      data.clear();
      data.set(args.data.getData(), args.data.size());

      locked = true;
      received = true; 
    };

    void onConnect( ofxLibwebsockets::Event& args )
    {};

    void onOpen( ofxLibwebsockets::Event& args )
    {};

    void onClose( ofxLibwebsockets::Event& args )
    {};

    void onIdle( ofxLibwebsockets::Event& args )
    {};

    void onBroadcast( ofxLibwebsockets::Event& args )
    {};

  private:

    ofxLibwebsockets::Client client;
    bool received, locked;

    ofBuffer data;

    ofPixels pix;
    ofTexture tex;

    void deserialize(ofBuffer& data, int pix_w, int pix_h, int pix_chan)
    {
      unsigned char* pixd = reinterpret_cast<unsigned char*>(data.getData());
      pix.setFromPixels(pixd, pix_w, pix_h, pix_chan);
      tex.loadData(pix);
    }; 
};

