#pragma once
#include "effect_base.h"


namespace toaster {

class EffectBlank : public FlexEffect {
public:
  EffectBlank(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }

  virtual bool isBlank() const {
    return true;
  }

};

};
