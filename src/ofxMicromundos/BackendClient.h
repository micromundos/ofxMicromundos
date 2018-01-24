#pragma once

#include "BinClient.h"
#include "MsgClient.h"

class BackendClient
{
  public:

    BackendClient() {};
    ~BackendClient() 
    {
      dispose();
    };

    void init(string ip, int port_bin, int port_msg)
    {
      msg.init(ip, port_msg); 
      bin.init(ip, port_bin);
    };

    void update()
    {
      msg.update();
      if (msg.pix_ready()) 
        bin.update(msg.pix_width(), msg.pix_height(), msg.pix_chan());
    };

    void dispose()
    {
      msg.dispose();
      bin.dispose();
    };

    void render_projected_pixels(float w, float h)
    {
      bin.render(0, 0, w, h);
    };

    void print_connection(float x, float y)
    {
      float lh = 24;
      bin.print_connection(x, y);
      msg.print_connection(x, y + lh);
    };

    void print_metadata(float x, float y)
    {
      msg.print_metadata(x, y);
    };

    void print_bloques(float x, float y)
    {
      msg.print_bloques(x, y);
    };

    ofPixels& projected_pixels()
    {
      return bin.pixels();
    };

    ofTexture& projected_texture()
    {
      return bin.texture();
    };

    map<int, Bloque>& projected_bloques()
    {
      return msg.bloques();
    };

    bool calib_enabled()
    {
      return msg.calib_enabled();
    };

  private:

    BinClient bin;
    MsgClient msg;
};

