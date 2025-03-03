#pragma once
#include "effect_base.h"


namespace toaster {

class EffectSidePanel : public FlexEffect {
public:
  EffectSidePanel(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
    uint8_t r = _personalColor[PC_SIDE][0];
    uint8_t g = _personalColor[PC_SIDE][1];
    uint8_t b = _personalColor[PC_SIDE][2];

    _colorFunc(0, 0, r, g, b, 255, PC_SIDE);
    display.fillHalf(0, r, g, b);

    r = _personalColor[PC_SIDE + PC_MAX][0];
    g = _personalColor[PC_SIDE + PC_MAX][1];
    b = _personalColor[PC_SIDE + PC_MAX][2];

    _colorFunc(HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN - 1, 0, r, g, b, 255, PC_SIDE + PC_MAX);
    display.fillHalf(1, r, g, b);
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }


};

};
