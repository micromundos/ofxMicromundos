#pragma once

#include "ofMain.h"
#include "ofxLibwebsockets.h"
#include "ofxMicromundos/Bloque.h"

class MsgClient
{
  public:

    MsgClient() {};
    ~MsgClient() {};

    void init(string host, int port)
    {
      client.connect(host, port);
      client.addListener(this);

      received = false;
      locked = false;

      _pix_w = 0;
      _pix_h = 0;
      _pix_chan = 0;
    };

    void dispose()
    {
      client.exit();
    };

    bool update()
    {
      if ( !received )
        return false;
      parse();
      received = false;
      locked = false;
      return true;
    };

    void print_info(float x, float y)
    {
      ofxLibwebsockets::Connection* conn = client.getConnection();
      string name = conn->getClientName();
      string ip = conn->getClientIP();
      string info = "msg connected: name="+name + " / ip=" + ip; 
      float lh = 24;
      ofDrawBitmapStringHighlight(info, x, y+lh/2);
    };

    void print_pix_data(float x, float y)
    {
      stringstream info;
      info << "msg pix" 
        << " w " << _pix_w
        << " h " << _pix_h 
        << " chan " << _pix_chan;
      float lh = 24;
      ofDrawBitmapStringHighlight(info.str(), x, y+lh/2);
    };

    void print_bloques(float x, float y)
    {
      float lh = 24;
      y += lh/2;
      ofDrawBitmapStringHighlight("bloques", x, y);
      y += lh;
      for (auto& bloque : _bloques)
      {
        Bloque& b = bloque.second;
        stringstream bstr;
        bstr
          << " id " << b.id
          << " loc " << b.loc;
          //<< " dir " << b.dir
          //<< " radio " << b.radio
          //<< " angle " << b.angle;
        ofDrawBitmapStringHighlight(bstr.str(), x, y);
        y += lh;
      }
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
    {};

    void onOpen( ofxLibwebsockets::Event& args )
    {};

    void onClose( ofxLibwebsockets::Event& args )
    {};

    void onIdle( ofxLibwebsockets::Event& args )
    {};

    void onBroadcast( ofxLibwebsockets::Event& args )
    {}; 

    int pix_width() { return _pix_w; };
    int pix_height() { return _pix_h; };
    int pix_chan() { return _pix_chan; };

    bool pix_ready()
    {
      return _pix_w != 0 && _pix_h != 0 && _pix_chan != 0;
    };

    map<int, Bloque>& bloques()
    {
      return _bloques;
    };

  private:

    ofxLibwebsockets::Client client;
    bool received, locked;

    string message;
    int _pix_w, _pix_h, _pix_chan;
    map<int, Bloque> _bloques;

    void parse()
    {
      vector<string> data = ofSplitString(message, "_");

      if (data.size() > 0)
        parse_pix_data(data[0]);

      if (data.size() > 1)
        parse_bloques(ofSplitString(ofSplitString(data[1], ":")[1], ";"), _bloques);
    };

    void parse_pix_data(string pix_data_str)
    {
      vector<string> p0 = ofSplitString(pix_data_str, ":");
      if (p0.size() > 1)
      {
        vector<string> pix_data = ofSplitString(p0[1], "#");
        ofVec2f dim = d2vec(pix_data[0]);
        _pix_w = dim.x;
        _pix_h = dim.y;
        _pix_chan = d2i(pix_data[1]);
      }
    };

    //see Backend->tags_to_bloques
    void parse_bloques(vector<string> bloques_str, map<int, Bloque>& bloques)
    {
      map<int,bool> cur;

      for (auto& b : bloques_str)
      {
        vector<string> bdata = ofSplitString(b, "#");
        if (bdata.size() == 0 || bdata[0].empty())
          continue;
        int id = d2i(bdata[0]);
        cur[id] = true;
        if (bloques.find(id) == bloques.end())
          make_bloque(id, bdata, bloques);
        else 
          update_bloque(id, bdata, bloques[id]);
      }
    };

    void make_bloque(int id, vector<string>& bdata, map<int, Bloque>& bloques)
    {
      Bloque b;

      set_bloque(id, bdata, b);

      b.loc_i = b.loc;
      b.dir_i = b.dir;
      b.angle_i = b.angle;

      bloques[b.id] = b;
    };

    void update_bloque(int id, vector<string>& bdata, Bloque& b)
    {
      interpolate_bloque(bdata, b);
      set_bloque(id, bdata, b); 
    };

    void set_bloque(int id, vector<string>& bdata, Bloque& b)
    {
      //TODO MsgClient: better de-serialization
      //int id = d2i(bdata[0]);
      ofVec2f loc = d2vec(bdata[1]);
      ofVec2f dir = d2vec(bdata[2]);
      float angle = d2f(bdata[3]);
      float radio = d2f(bdata[4]);

      b.id = id;
      b.loc = loc;
      b.dir = dir;
      b.angle = angle;
      b.radio = radio;

      //if (bdata.size() > 4)
      //{
        //b.corners[0] = d2vec(bdata[4]);
        //b.corners[1] = d2vec(bdata[5]);
        //b.corners[2] = d2vec(bdata[6]);
        //b.corners[3] = d2vec(bdata[7]);
      //}
    };

    void interpolate_bloque(vector<string>& bdata, Bloque& b)
    {
      ofVec2f loc = d2vec(bdata[1]);
      ofVec2f dir = d2vec(bdata[2]);
      int angle = d2f(bdata[3]);

      b.loc_i += (loc - b.loc_i) * 0.2;
      b.dir_i += (dir - b.dir_i) * 0.2;
      b.angle_i = ofLerpRadians(b.angle_i, angle, 0.05);
    };

    //deserialization

    int d2i(string d)
    {
      return stoi(ofSplitString(d, "=")[1]);
    };

    float d2f(string d)
    {
      return stof(ofSplitString(d, "=")[1]);
    };

    ofVec2f d2vec(string d)
    {
      vector<string> vec = ofSplitString(ofSplitString(d, "=")[1], ",");
      return ofVec2f(stof(vec[0]), stof(vec[1]));
    };
};

