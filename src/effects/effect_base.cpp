#include "effect_base.h"
#include "effect_script.h"

#include "effects.h"


namespace toaster {

std::vector<Effect*> Effect::_effects;
timer_us_t Effect::_sync_timer_us = 0;

bool Effect::_staticMode = false;
timer_us_t Effect::_static_mode_tick_us = 0;
timer_us_t Effect::_color_tick_us = 0;
PROTOGEN_COLOR_MODE Effect::_colorMode = PCM_DEFAULT;
COLOR_FUNC Effect::_colorFunc = color_func_personal;
uint8_t Effect::_personalColor[8][3] = {
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
};
uint32_t Effect::_rainbowSingleSpeed_ms = 20;
uint32_t Effect::_rainbowSpeed_ms = 10;
uint32_t Effect::_rainbowTransitionPixels = 4;
LinearCalibrate<float, uint32_t> Effect::_gradation_r[2];
LinearCalibrate<float, uint32_t> Effect::_gradation_g[2];
LinearCalibrate<float, uint32_t> Effect::_gradation_b[2];
bool Effect::_rainbow_single_dirty = false;
uint8_t Effect::_rainbow_single_red = 0;
uint8_t Effect::_rainbow_single_green = 0;
uint8_t Effect::_rainbow_single_blue = 0;
bool Effect::_rainbow_dirty = false;
uint8_t Effect::_rainbow_red[HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_rainbow_green[HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_rainbow_blue[HUB75_PANEL_RES_X] = {0, };
bool Effect::_gradation_dirty = false;
uint8_t Effect::_gradation_reds[2][HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_gradation_greens[2][HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_gradation_blues[2][HUB75_PANEL_RES_X] = {0, };


EffectScript effect_script[6]{"script0", "script1", "script2", "script3", "script4", "script5"};

EffectBlank flex_blank("blank");
EffectWhite flex_white("white");
EffectRainbow flex_rainbow("rainbow");
EffectFestive effect_festive("festive");
EffectDino effect_dino("dino");
EffectClock effect_clock("clock");

EffectSidePanel side_default("side_default");
EffectSideRainbow side_rainbow("side_rainbow");




Effect::Effect(const char* name) {
  _name = name;
  _effects.push_back(this);
}


Effect::~Effect() {
  for (auto it = _effects.begin(); it != _effects.end(); ) {
    if ((*it) == this) {
      it = _effects.erase(it);
    }
    else {
      ++it;
    }
  }
}


Effect* Effect::find(const char* name, uint8_t script_index, const char* base_path) {
  if (*name == 0) {
    return nullptr;
  } 
  
  auto result = findFromHardcoded(name);
  if (result != nullptr) {
    return result;
  }

  return findFromScript(name, script_index, base_path);
}


Effect* Effect::findFromHardcoded(const char* name) {
  for (auto it = _effects.begin(); it != _effects.end(); ++it) {
    if (strcasecmp((*it)->_name, name) == 0) {
      return *it;
    }
  }

  return nullptr;
}


Effect* Effect::findFromScript(const char* name, uint8_t script_index, const char* base_path) {
  auto& script = effect_script[script_index];

  if (script.loadScript(name, base_path)) {
    return &script;
  }

  return nullptr;
}


void Effect::h2rgb(uint16_t h, uint8_t& out_r, uint8_t& out_g, uint8_t& out_b) {
  uint16_t region = h / 60;
  uint16_t remainder = (h - (region * 60)) * 6;
  
  uint8_t q = (255 * (255 - ((255 * remainder) / 360))) >> 8;
  uint8_t t = (255 * (255 - ((255 * (359 - remainder)) / 360))) >> 8;
  
  switch (region) {
  case 0:
    out_r = 255; out_g = t; out_b = 0;
    break;
  case 1:
    out_r = q; out_g = 255; out_b = 0;
    break;
  case 2:
    out_r = 0; out_g = 255; out_b = t;
    break;
  case 3:
    out_r = 0; out_g = q; out_b = 255;
    break;
  case 4:
    out_r = t; out_g = 0; out_b = 255;
    break;
  default:
    out_r = 255; out_g = 0; out_b = q;
    break;
  }
}


timer_ms_t Effect::color_millis() {
  if (_staticMode) {
    return (_static_mode_tick_us - _color_tick_us) / 1000;
  }

  return (Timer::get_micros() - _color_tick_us) / 1000;
}


void Effect::color_func_original(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
}

void Effect::color_func_personal(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (param >= PC_MAX) {
    return;
  }

  r = _personalColor[param][0];
  g = _personalColor[param][1];
  b = _personalColor[param][2];
}

void Effect::color_func_rainbow_single(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  if (_rainbow_single_dirty) {
    _rainbow_single_dirty = false;
    int hue = color_millis() / _rainbowSingleSpeed_ms % 360;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    h2rgb(hue, color_r, color_g, color_b);
    _rainbow_single_red = color_r;
    _rainbow_single_green = color_g;
    _rainbow_single_blue = color_b;
  }

  r = _rainbow_single_red;
  g = _rainbow_single_green;
  b = _rainbow_single_blue;
}

void Effect::color_func_rainbow(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  if (_rainbow_dirty) {
    _rainbow_dirty = false;
    for (int i = 0; i < HUB75_PANEL_RES_X; i++) {
      int hue = (color_millis() / _rainbowSpeed_ms - (i * _rainbowTransitionPixels)) % 360;
      uint8_t color_r;
      uint8_t color_g;
      uint8_t color_b;
      h2rgb(hue, color_r, color_g, color_b);
      _rainbow_red[i] = color_r;
      _rainbow_green[i] = color_g;
      _rainbow_blue[i] = color_b;
    }
  }

  int x1 = (x < HUB75_PANEL_RES_X) ? x : (HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN - x - 1);

  r = _rainbow_red[x1];
  g = _rainbow_green[x1];
  b = _rainbow_blue[x1];
}

void Effect::color_func_gradation(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  int g_index = 0;

  if (_gradation_dirty) {
    _gradation_dirty = false;
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < HUB75_PANEL_RES_X; j++) {
        uint32_t color_r;
        uint32_t color_g;
        uint32_t color_b;
        _gradation_r[i].getValue((float)j / HUB75_PANEL_RES_X, color_r);
        _gradation_g[i].getValue((float)j / HUB75_PANEL_RES_X, color_g);
        _gradation_b[i].getValue((float)j / HUB75_PANEL_RES_X, color_b);
        _gradation_reds[i][j] = (uint8_t)color_r;
        _gradation_greens[i][j] = (uint8_t)color_g;
        _gradation_blues[i][j] = (uint8_t)color_b;
      }
    }
  }
  
  int x1 = (x < HUB75_PANEL_RES_X) ? x : (HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN - x - 1);

  r = _gradation_reds[g_index][x1];
  g = _gradation_greens[g_index][x1];
  b = _gradation_blues[g_index][x1];
}

void Effect::color_func_gray_personal(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (param >= PC_MAX) {
    return;
  }

  // Rec. 601
  uint8_t gray = (uint8_t)(((2990 * r) + (5870 * g) + (1140 * b)) / 10000);
  r = (uint8_t)((uint16_t)_personalColor[param][0] * gray / 255);
  g = (uint8_t)((uint16_t)_personalColor[param][1] * gray / 255);
  b = (uint8_t)((uint16_t)_personalColor[param][2] * gray / 255);
}

void Effect::color_func_gray_rainbow_single(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  if (_rainbow_single_dirty) {
    _rainbow_single_dirty = false;
    int hue = color_millis() / _rainbowSingleSpeed_ms % 360;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    h2rgb(hue, color_r, color_g, color_b);
    _rainbow_single_red = color_r;
    _rainbow_single_green = color_g;
    _rainbow_single_blue = color_b;
  }

  // Rec. 601
  uint8_t gray = (uint8_t)(((2990 * r) + (5870 * g) + (1140 * b)) / 10000);
  r = (uint8_t)((uint16_t)_rainbow_single_red * gray / 255);
  g = (uint8_t)((uint16_t)_rainbow_single_green * gray / 255);
  b = (uint8_t)((uint16_t)_rainbow_single_blue * gray / 255);
}

void Effect::color_func_gray_rainbow(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  if (_rainbow_dirty) {
    _rainbow_dirty = false;
    for (int i = 0; i < HUB75_PANEL_RES_X; i++) {
      int hue = (color_millis() / _rainbowSpeed_ms - (i * _rainbowTransitionPixels)) % 360;
      uint8_t color_r;
      uint8_t color_g;
      uint8_t color_b;
      h2rgb(hue, color_r, color_g, color_b);
      _rainbow_red[i] = color_r;
      _rainbow_green[i] = color_g;
      _rainbow_blue[i] = color_b;
    }
  }

  int x1 = (x < HUB75_PANEL_RES_X) ? x : (HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN - x - 1);

  // Rec. 601
  uint8_t gray = (uint8_t)(((2990 * r) + (5870 * g) + (1140 * b)) / 10000);
  r = (uint8_t)((uint16_t)_rainbow_red[x1] * gray / 255);
  g = (uint8_t)((uint16_t)_rainbow_green[x1] * gray / 255);
  b = (uint8_t)((uint16_t)_rainbow_blue[x1] * gray / 255);
}

void Effect::color_func_gray_gradation(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  int g_index = 0;

  if (_gradation_dirty) {
    _gradation_dirty = false;
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < HUB75_PANEL_RES_X; j++) {
        uint32_t color_r;
        uint32_t color_g;
        uint32_t color_b;
        _gradation_r[i].getValue((float)j / HUB75_PANEL_RES_X, color_r);
        _gradation_g[i].getValue((float)j / HUB75_PANEL_RES_X, color_g);
        _gradation_b[i].getValue((float)j / HUB75_PANEL_RES_X, color_b);
        _gradation_reds[i][j] = (uint8_t)color_r;
        _gradation_greens[i][j] = (uint8_t)color_g;
        _gradation_blues[i][j] = (uint8_t)color_b;
      }
    }
  }
  
  int x1 = (x < HUB75_PANEL_RES_X) ? x : (HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN - x - 1);

  // Rec. 601
  uint8_t gray = (uint8_t)(((2990 * r) + (5870 * g) + (1140 * b)) / 10000);
  r = (uint8_t)((uint16_t)_gradation_reds[g_index][x1] * gray / 255);
  g = (uint8_t)((uint16_t)_gradation_greens[g_index][x1] * gray / 255);
  b = (uint8_t)((uint16_t)_gradation_blues[g_index][x1] * gray / 255);
}

};
