#pragma once
#include "effect_base.h"
#include "protogen.h"


namespace toaster {

class EffectFestive : public FixedEffect {
public:
  EffectFestive(const char* name) : FixedEffect(name) {
  }
  
  virtual void init(Display& display) {
    Effect::init(display);

    _vdatas = new VDATA[display.getWidth() / HUB75_PANEL_CHAIN];
    memset(_vdatas, 0, sizeof(_vdatas));
    
    if (Protogen.isFestiveFace()) {
      _eyes[0] = display.load_png("/png/eyes/eye_default_0.png");
      _eyes[1] = display.load_png("/png/eyes/eye_default_1.png");
      _eyes[2] = display.load_png("/png/eyes/eye_default_2.png");
      _eyes[3] = display.load_png("/png/eyes/eye_default_3.png");
      _nose = display.load_png("/png/nose/nose_default.png");
      _mouth = display.load_png("/png/mouth/mouth_default.png");
    }

    _eye_frame = 0;
  }

  virtual void process(Display& display) {
    const int EASE_MAX = 6;

    if (_staticMode == false) {
      for (int x = 0; x < display.getWidth() / 2; x++) {
        if (random(10) == 0) {
          int newHeight = gaussianRandom(display.getHeight() + 1);
          // int newHeight = random(display.getHeight() + 1);
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

    switch (_step) {
    case 0:
      _eye_frame = 0;
      if (timeout(3000)) {
        setStep(100);
        break;
      }
      break;
    case 100:
      _eye_frame = 1;
      if (timeout(50)) {
        setStep(200);
        break;
      }
      break;
    case 200:
      _eye_frame = 2;
      if (timeout(50)) {
        setStep(300);
        break;
      }
      break;
    case 300:
      _eye_frame = 3;
      if (timeout(50)) {
        setStep(400);
        break;
      }
      break;
    case 400:
      _eye_frame = 2;
      if (timeout(50)) {
        setStep(500);
        break;
      }
      break;
    case 500:
      _eye_frame = 1;
      if (timeout(50)) {
        setStep(0);
        break;
      }
      break;
    }

    if (Protogen.isFestiveFace()) {
      display.draw_png_newcolor(_eyes[_eye_frame], color_func_rainbow_single, 0, DRAW_MIRROR, 0, 0);
      display.draw_png_newcolor(_nose, color_func_rainbow_single, 1, DRAW_MIRROR, 48, 0);
      display.draw_png_newcolor(_mouth, color_func_rainbow_single, 2, DRAW_MIRROR, 0, 16);
    }
  }

  virtual void release(Display& display) {
    delete[] _vdatas;
    _vdatas = nullptr;
    
    for (int i = 0; i < 4; i++) {
      if (_eyes[i] != nullptr) {
        display.unload_png(_eyes[i]);
        _eyes[i] = nullptr;
      }
    }
    
    if (_nose != nullptr) {
      display.unload_png(_nose);
      _nose = nullptr;
    }

    if (_mouth != nullptr) {
      display.unload_png(_mouth);
      _mouth = nullptr;
    }

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

protected:
  upng_t* _eyes[4]{nullptr,};
  upng_t* _nose{nullptr};
  upng_t* _mouth{nullptr};
  uint8_t _eye_frame{0};

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
