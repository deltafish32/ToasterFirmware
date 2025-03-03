#include "effect_festive.h"

#include "lib/logger.h"
#include "protogen.h"


namespace toaster {


typedef struct _FRAME_DATA {
  uint8_t image;
  uint32_t display_time;
} FRAME_DATA;

static const FRAME_DATA FRAME_DATAS[] = {
  {0, 3000, },
  {1, 16, },
  {2, 16, },
  {3, 16, },
  {4, 16, },
  {5, 100, },
  {4, 16, },
  {3, 16, },
  {2, 16, },
  {1, 16, },
};

static const size_t FRAME_DATA_COUNT = sizeof(FRAME_DATAS) / sizeof(FRAME_DATAS[0]);


void EffectFestive::init(Display& display) {
  Effect::init(display);

  _vdatas = new VDATA[display.getWidth() / HUB75_PANEL_CHAIN];
  if (_vdatas == nullptr) {
    TF_LOGE("EffectFestive", "memory allocation failed.");
    return;
  }
  memset(_vdatas, 0, sizeof(_vdatas));
  
  if (Protogen.isFestiveFace()) {
    bool use_rgb565 = Protogen.isRGB565();
    std::string base_path = Toaster::DEFAULT_BASE_PATH;
    _eyes[0] = new Image((base_path + "/png/eyes/eye_default_0.png").c_str(), use_rgb565);
    _eyes[1] = new Image((base_path + "/png/eyes/eye_default_1.png").c_str(), use_rgb565);
    _eyes[2] = new Image((base_path + "/png/eyes/eye_default_2.png").c_str(), use_rgb565);
    _eyes[3] = new Image((base_path + "/png/eyes/eye_default_3.png").c_str(), use_rgb565);
    _eyes[4] = new Image((base_path + "/png/eyes/eye_default_4.png").c_str(), use_rgb565);
    _eyes[5] = new Image((base_path + "/png/eyes/eye_default_5.png").c_str(), use_rgb565);
    _nose = new Image((base_path + "/png/nose/nose_default.png").c_str(), use_rgb565);
    _mouth = new Image((base_path + "/png/mouth/mouth_default.png").c_str(), use_rgb565);
  }

  _eye_frame = 0;
}


void EffectFestive::process(Display& display) {
  if (_vdatas == nullptr) {
    return;
  }
  
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

  if (Protogen.isFestiveFace()) {
    _eye_frame = FRAME_DATAS[_step].image;
    if (timeout(FRAME_DATAS[_step].display_time)) {
      setStep((_step >= FRAME_DATA_COUNT - 1) ? 0 : _step + 1);
    }
    
    display.draw_image_newcolor(_eyes[_eye_frame], color_func_rainbow_single, 0, DRAW_MIRROR, 0, 0);
    display.draw_image_newcolor(_nose, color_func_rainbow_single, 1, DRAW_MIRROR, 48, 0);
    display.draw_image_newcolor(_mouth, color_func_rainbow_single, 2, DRAW_MIRROR, 0, 16);
  }
}


void EffectFestive::release(Display& display) {
  if (_vdatas != nullptr) {
    delete[] _vdatas;
    _vdatas = nullptr;
  }
  
  for (int i = 0; i < 6; i++) {
    if (_eyes[i] != nullptr) {
      delete _eyes[i];
      _eyes[i] = nullptr;
    }
  }
  
  if (_nose != nullptr) {
    delete _nose;
    _nose = nullptr;
  }

  if (_mouth != nullptr) {
    delete _mouth;
    _mouth = nullptr;
  }

  Effect::release(display);
}


};
