#pragma once
#include "effect_base.h"


namespace toaster {

class EffectSideAlign : public FlexEffect {
public:
  EffectSideAlign(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
    for (int i = 0; i < display.getHeight(); i++) {
      for (int j = 0; j < display.getWidth(); j += display.getWidth() / 2) {
        uint8_t r = _personalColor[PC_SIDE - PC_EYES][0];
        uint8_t g = _personalColor[PC_SIDE - PC_EYES][1];
        uint8_t b = _personalColor[PC_SIDE - PC_EYES][2];
        _colorFunc(0, 0, r, g, b, 255, PC_SIDE);
        display.setPixel(j, i, r, g, b);
      }
    }
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }


};

};
