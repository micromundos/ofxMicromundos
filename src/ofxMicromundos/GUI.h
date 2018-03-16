#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class GUI
{
  public:

    ofParameter<bool> backend_monitor;
    ofParameter<bool> print_bloques;
    ofParameter<bool> send;

    void init_params()
    {
      p.add( backend_monitor
          .set("backend_monitor", true) );

      p.add( print_bloques
          .set("print_bloques", true) );

      p.add( send
          .set("send", true) ); 
    }; 

    void init(float w)
    {
      ofxBaseGui::setDefaultWidth(w);
      p.setName("params");
      init_params();
      gui.setup(p, "backend_gui.xml");
      gui.loadFromFile("backend_gui.xml");
    }; 

    void render(float x, float y)
    { 
      gui.setPosition(x, y);
      gui.draw();
    };

  private:

    ofParameterGroup p;
    ofxPanel gui; 
};

