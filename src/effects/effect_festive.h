#pragma once
#include "effect_base.h"


namespace toaster {

class EffectFestive : public FixedEffect {
public:
  EffectFestive(const char* name) : FixedEffect(name) {
  }
  
  virtual void init(Display& display) {
    Effect::init(display);

    _vdatas = new VDATA[display.getWidth() / 2];
    memset(_vdatas, 0, sizeof(_vdatas));
  }

  virtual void process(Display& display) {
    const int EASE_MAX = 6;

    switch (_step) {
    case 0:
      if (_staticMode == false) {
        for (int x = 0; x < display.getWidth() / 2; x++) {
          if (random(10) == 0) {
            int newHeight = gaussianRandom(display.getHeight() + 1);
            if (_vdatas[x].heightRecent < newHeight) {
              _vdatas[x].heightPrev = _vdatas[x].heightRecent;
              _vdatas[x].heightNext = newHeight;
              _vdatas[x].ease = 0;
              _vdatas[x].delay = 12;
            }
          }
        }
      }

      for (int x = 0; x < display.getWidth() / 2; x++) {
        if (_vdatas[x].ease < EASE_MAX) {
          _vdatas[x].ease += 1;
          _vdatas[x].heightRecent = (_vdatas[x].heightPrev * (EASE_MAX - _vdatas[x].ease) + _vdatas[x].heightNext * _vdatas[x].ease) / EASE_MAX;
        }
        else {
          if (_vdatas[x].delay > 0) {
            _vdatas[x].delay -= 1;
          }
          else if (_vdatas[x].heightRecent > 0) {
            _vdatas[x].heightRecent -= 1;
          }
        }
      }
      
      for (int x = 0; x < display.getWidth() / 2; x++) {
        for (int y = display.getHeight() - _vdatas[x].heightRecent; y < display.getHeight(); y++) {
          uint8_t r, g, b;
          color_func_rainbow(x, y, r, g, b, 255, 0);
          display.setPixel(x, y, r, g, b);

          int x2 = display.getWidth() - x - 1;
          display.setPixel(x2, y, r, g, b);
        }
      }

      break;
    }
  }

  virtual void release(Display& display) {
    delete[] _vdatas;
    _vdatas = nullptr;
    
    Effect::release(display);
  }


protected:
  typedef struct _VDATA{
    int heightPrev{0};
    int heightNext{0};
    int heightRecent{0};
    int ease{0};
    int delay{0};
  } VDATA;

  VDATA* _vdatas{nullptr};

};


class EffectFestiveSidePanel : public FlexEffect {
public:
  EffectFestiveSidePanel(const char* name) : FlexEffect(name) {
  }

  virtual void init(Display& display) {
    Effect::init(display);
  }

  virtual void process(Display& display) {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    color_func_rainbow(0, 0, r, g, b, 255, PC_SIDE);
    display.fill(r, g, b);
  }

  virtual void release(Display& display) {
    
    Effect::release(display);
  }

};


};
