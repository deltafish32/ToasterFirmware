#pragma once
#include "effect_base.h"


namespace toaster {


class EffectClock : public FixedEffect {
public:
  EffectClock(const char* name) : FixedEffect(name) {
  }
  
  virtual void init(Display& display);
  virtual void process(Display& display);
  virtual void release(Display& display);

protected:
  Image* _image_clock{nullptr,};
  uint8_t _hour{0};
  uint8_t _minute{0};
  // uint8_t _second{0};
  uint8_t _blink{0};

protected:
  void sync();
};

};
