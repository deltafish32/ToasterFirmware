#pragma once
#include "effect_base.h"


namespace toaster {

class EffectWhite : public FlexEffect {
public:
  EffectWhite(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
    display.fillWhite();
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }


};

};
