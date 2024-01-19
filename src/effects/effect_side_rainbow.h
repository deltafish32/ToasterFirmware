#pragma once
#include "effect_base.h"


namespace toaster {

class EffectSideRainbow : public FlexEffect {
public:
  EffectSideRainbow(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
    const int GROUP = 3;
    const int RAINBOW_SPEED = 1;
    const int RAINBOW_TRANSITION = 64;
    const int OFF_SPEED = 40;

    size_t width = display.getWidth();
    size_t numleds = width / 2;

    int off_index = (color_millis() / OFF_SPEED) % numleds;

    for (int i = 0; i < display.getHeight(); i++) {
      for (int j = 0; j < numleds / GROUP; j++) {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        uint32_t hue = (color_millis() / _rainbowSpeed_ms - (j * RAINBOW_TRANSITION)) % 360;
        h2rgb(hue, r, g, b);

        for (int k = 0; k < GROUP; k++) {
          int index = (j * GROUP) + k;
          if (index == off_index) {
            continue;
          }

          display.setPixel(index, i, r, g, b);
          display.setPixel(index + numleds, i, r, g, b);
        }
      }
    }
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }


};

};
