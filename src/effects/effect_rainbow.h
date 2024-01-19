#pragma once
#include "effect_base.h"


namespace toaster {

class EffectRainbow : public FlexEffect {
public:
  EffectRainbow(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
    int hue = color_millis() / _rainbowSingleSpeed_ms % 360;
    uint8_t r, g, b;
    h2rgb(hue, r, g, b);
    display.fill(r, g, b);
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }


};

};
