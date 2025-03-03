#pragma once
#include "effect_base.h"
#include "protogen.h"
#include "lib/logger.h"


namespace toaster {

class EffectFestive : public FixedEffect {
public:
  EffectFestive(const char* name) : FixedEffect(name) {
  }
  
  virtual void init(Display& display);
  virtual void process(Display& display);
  virtual void release(Display& display);

protected:
  typedef struct _VDATA {
    int heightPrev{0};
    int heightNext{0};
    int heightRecent{0};
    int ease{0};
    int delay{0};
  } VDATA;

  VDATA* _vdatas{nullptr};

protected:
  Image* _eyes[6]{nullptr,};
  Image* _nose{nullptr};
  Image* _mouth{nullptr};
  uint8_t _eye_frame{0};

};


};
