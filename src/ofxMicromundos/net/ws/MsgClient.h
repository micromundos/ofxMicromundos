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

      _pix_w = 0;
      _pix_h = 0;
      _pix_chan = 0;
      _binary_enabled = false;
      _syphon_enabled = false;
      _calib_enabled = false;
      _juego_active = "";
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

    void print_connection(float x, float& y, float LH)
    {
      ofxLibwebsockets::Connection* conn = client.getConnection();
      if (conn == nullptr)
      {
        string info = "msg not connected"; 
        ofDrawBitmapStringHighlight(info, x, y);
        y += LH;
        return;
      }

      string name = conn->getClientName();
      string ip = conn->getClientIP();
      string info = "msg connected: name="+name + " / ip=" + ip; 
      ofDrawBitmapStringHighlight(info, x, y);
      y += LH;
    };

    void print_metadata(float x, float& y, float LH)
    {
      stringstream msg;

      msg << "metadata= "
        << "\n"
        << " pixels:" 
          << " dim " << _pix_w << "," << _pix_h 
          << " chan " << _pix_chan 
        << "\n"
        << " net:"
          << " binary " << _binary_enabled
          << " syphon " << _syphon_enabled
        << "\n"
        << " calib:" 
          << " enabled " << _calib_enabled
        << "\n"
        << " juegos:"
          << " active " << _juego_active;

      ofDrawBitmapStringHighlight(msg.str(), x, y);

      float lines = 5;
      y += LH * lines;
    };

    void print_bloques(float x, float& y, float LH)
    {
      ofDrawBitmapStringHighlight("bloques", x, y);
      y += LH;

      for (const auto& bloque : _bloques)
      {
        const Bloque& b = bloque.second;
        stringstream bstr;
        bstr
          << " id " << b.id
          << " loc " << b.loc;
          //<< " dir " << b.dir
          //<< " radio " << b.radio
          //<< " angle " << b.angle;
        ofDrawBitmapStringHighlight(bstr.str(), x, y);
        y += LH;
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

    int pix_width() { return _pix_w; };
    int pix_height() { return _pix_h; };
    int pix_chan() { return _pix_chan; };

    bool binary_enabled() { return _binary_enabled; }
    bool syphon_enabled() { return _syphon_enabled; }
    bool calib_enabled() { return _calib_enabled; };

    string juego_active() 
    { 
      return _juego_active; 
    };

    bool juego_active(string name) 
    { 
      return _juego_active.compare(name) == 0; 
    };

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
    bool _binary_enabled, _syphon_enabled;
    bool _calib_enabled;
    string _juego_active;
    map<int, Bloque> _bloques;

    void deserialize(string message)
    {
      vector<string> data = ofSplitString(message, "_");

      if (data.size() > 0)
        parse_pix_data(data[0]);

      if (data.size() > 1)
        parse_net_data(data[1]);

      if (data.size() > 2)
        parse_calib_data(data[2]);

      if (data.size() > 3)
        parse_juegos_data(data[3]);

      if (data.size() > 4)
        parse_bloques(data[4], _bloques);
    };

    void parse_pix_data(string data_str)
    {
      vector<string> data = ofSplitString(data_str, ":");
      if (data.size() > 1)
      {
        vector<string> d = ofSplitString(data[1], "#");
        ofVec2f dim = d2vec(d[0]);
        _pix_w = dim.x;
        _pix_h = dim.y;
        _pix_chan = d2i(d[1]);
      }
    }; 

    void parse_net_data(string data_str)
    {
      vector<string> data = ofSplitString(data_str, ":");
      if (data.size() > 1)
      {
        vector<string> d = ofSplitString(data[1], "#");
        _binary_enabled = bool(d2i(d[0]));
        _syphon_enabled = bool(d2i(d[1]));
      }
    };

    void parse_calib_data(string data_str)
    {
      vector<string> data = ofSplitString(data_str, ":");
      if (data.size() > 1)
      {
        vector<string> d = ofSplitString(data[1], "#");
        _calib_enabled = bool(d2i(d[0]));
      }
    };

    void parse_juegos_data(string data_str)
    {
      vector<string> data = ofSplitString(data_str, ":");
      if (data.size() > 1)
      {
        vector<string> d = ofSplitString(data[1], "#");
        _juego_active = d2s(d[0]);
      }
    };

    //see Backend->tags_to_bloques
    void parse_bloques(string data_str, map<int, Bloque>& bloques)
    {
      vector<string> bloques_str = ofSplitString(ofSplitString(data_str, ":")[1], ";");

      map<int,bool> cur;

      for (const auto& b : bloques_str)
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

      vector<int> remove;
      for (const auto& bloque : bloques)
      {
        int id = bloque.first;
        if (cur.find(id) == cur.end())
          remove.push_back(id);
      }
      for (const auto& id : remove)
        bloques.erase(id);
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
      //TODO MsgClient: better deserialization
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

    string d2s(string d)
    {
      return ofSplitString(d, "=")[1];
    };

    ofVec2f d2vec(string d)
    {
      vector<string> vec = ofSplitString(ofSplitString(d, "=")[1], ",");
      return ofVec2f(stof(vec[0]), stof(vec[1]));
    };
};

