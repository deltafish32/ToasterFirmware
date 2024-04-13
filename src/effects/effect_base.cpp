#include "effect_base.h"
#include "effect_script.h"

#include "effects.h"


namespace toaster {

std::vector<Effect*> Effect::_effects;

bool Effect::_staticMode = false;
timer_us_t Effect::_static_mode_tick_us = 0;
timer_us_t Effect::_color_tick_us = 0;
PROTOGEN_COLOR_MODE Effect::_colorMode = PCM_DEFAULT;
COLOR_FUNC Effect::_colorFunc = color_func_personal;
uint8_t Effect::_personalColor[4][3] = {
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
  {0, 255, 0, },
};
uint32_t Effect::_rainbowSingleSpeed_ms = 20;
uint32_t Effect::_rainbowSpeed_ms = 10;
uint32_t Effect::_rainbowTransitionPixels = 4;
LinearCalibrate<float, uint32_t> Effect::_gradation_r;
LinearCalibrate<float, uint32_t> Effect::_gradation_g;
LinearCalibrate<float, uint32_t> Effect::_gradation_b;
bool Effect::_rainbow_single_dirty = false;
uint8_t Effect::_rainbow_single_red = 0;
uint8_t Effect::_rainbow_single_green = 0;
uint8_t Effect::_rainbow_single_blue = 0;
bool Effect::_rainbow_dirty = false;
uint8_t Effect::_rainbow_red[HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_rainbow_green[HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_rainbow_blue[HUB75_PANEL_RES_X] = {0, };
bool Effect::_gradation_dirty = false;
uint8_t Effect::_gradation_reds[HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_gradation_greens[HUB75_PANEL_RES_X] = {0, };
uint8_t Effect::_gradation_blues[HUB75_PANEL_RES_X] = {0, };


EffectScript effect_script[4]{"script0", "script1", "script2", "script3"};

EffectBlank flex_blank("blank");
EffectWhite flex_white("white");
EffectRainbow flex_rainbow("rainbow");
EffectFestive effect_festive("festive");
EffectFestiveSidePanel side_festive("side_festive");

EffectSidePanel side_default("side_default");
EffectSideRainbow side_rainbow("side_rainbow");




Effect* Effect::findFromScript(const char* name, uint8_t script_index) {
  auto& script = effect_script[script_index];

  if (script.loadScript(name)) {
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
  r = _personalColor[param - PC_EYES][0];
  g = _personalColor[param - PC_EYES][1];
  b = _personalColor[param - PC_EYES][2];
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

  r = _rainbow_red[x];
  g = _rainbow_green[x];
  b = _rainbow_blue[x];
}

void Effect::color_func_gradation(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param) {
  if (a == 0) {
    r = g = b = 0;
    return;
  }

  if (_gradation_dirty) {
    _gradation_dirty = false;
    for (int i = 0; i < HUB75_PANEL_RES_X; i++) {
      uint32_t color_r;
      uint32_t color_g;
      uint32_t color_b;
      _gradation_r.getValue((float)i / HUB75_PANEL_RES_X, color_r);
      _gradation_g.getValue((float)i / HUB75_PANEL_RES_X, color_g);
      _gradation_b.getValue((float)i / HUB75_PANEL_RES_X, color_b);
      _gradation_reds[i] = (uint8_t)color_r;
      _gradation_greens[i] = (uint8_t)color_g;
      _gradation_blues[i] = (uint8_t)color_b;
    }
  }
  
  r = _gradation_reds[x];
  g = _gradation_greens[x];
  b = _gradation_blues[x];
}

};
