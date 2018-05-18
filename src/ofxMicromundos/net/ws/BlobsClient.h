#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"

class BlobsClient
{
  public:

    BlobsClient() {};
    ~BlobsClient() {};

    void init(string host, int port)
    {
      if (port <= 0)
        return;

      vector<string> addr = ofSplitString(host, "//");
      host = addr.size() == 1 ? addr[0] : addr[1];

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
    };

    bool update()
    {
      if ( !received )
        return false;
      deserialize(message);
      received = false;
      locked = false;
      return true;
    };

    void print_connection(float x, float y)
    {
      ofxLibwebsockets::Connection* conn = client.getConnection();
      if (conn == nullptr)
        return;
      string name = conn->getClientName();
      string ip = conn->getClientIP();
      string info = "blobs connected: name="+name + " / ip=" + ip;
      float lh = 24;
      ofDrawBitmapStringHighlight(info, x, y+lh/2);
    };

    bool connected()
    {
      return client.isConnected();
    };

    void onMessage( ofxLibwebsockets::Event& args )
    {
      if ( args.isBinary || locked )
        return;
      message = args.message;
      locked = true;
      received = true;
    };

    void onConnect( ofxLibwebsockets::Event& args )
    {
      //ofLog() << "on connect";
    };

    void onOpen( ofxLibwebsockets::Event& args )
    {
      //ofLog() << "on open";
    };

    void onClose( ofxLibwebsockets::Event& args )
    {
      //ofLog() << "on close";
    };

    void onIdle( ofxLibwebsockets::Event& args )
    {
      //ofLog() << "on idle";
    };

    void onBroadcast( ofxLibwebsockets::Event& args )
    {}; 

    vector<ofPolyline>& get()
    {
      return _blobs;
    };

  private:

    ofxLibwebsockets::Client client;
    bool received, locked;

    string message;
    vector<ofPolyline> _blobs;

    void deserialize(string message)
    {
      vector<string> data = ofSplitString(message, ":");

      if (data.size() < 2)
        return;

      _blobs.clear();

      vector<string> blobs_str = ofSplitString(data[1], "#");
      for (const auto& blob_str : blobs_str)
      {
        ofPolyline polyline;
        vector<string> pts_str = ofSplitString(blob_str, ";");
        for (const auto& pt_str : pts_str)
        {
          vector<string> vec = ofSplitString(pt_str, ",");
          if (vec.size() < 2)
            continue;
          polyline.addVertex(stof(vec[0]), stof(vec[1]), 0);
        }
        polyline.close();
        _blobs.push_back(polyline);
      }
    };
};

