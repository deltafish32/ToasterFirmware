#pragma once
#include "effect_base.h"


namespace toaster {

class EffectSideRainbow : public FlexEffect {
public:
  EffectSideRainbow(const char* name);

  virtual void init(Display& display);
  virtual void process(Display& display);
  virtual void release(Display& display);

protected:
  virtual void process_strip(Display& display);
  virtual void process_circle(Display& display);
};

};
