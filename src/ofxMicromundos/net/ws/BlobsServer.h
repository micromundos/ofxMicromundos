#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"

class BlobsServer
{
  public:

    BlobsServer() {};
    ~BlobsServer() 
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

    bool send(vector<ofPolyline>& blobs, bool enabled)
    {
      if (!enabled || !connected())
        return false;
      _server.send(serialize(blobs));
      return true;
    };

    //blobs:0,0;1,1;2,2#0,0;1,1;2,2
    string serialize(vector<ofPolyline>& blobs)
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

