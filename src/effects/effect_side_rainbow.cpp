#include "effect_side_rainbow.h"

#include "lib/logger.h"
#include "protogen.h"


namespace toaster {


EffectSideRainbow::EffectSideRainbow(const char* name) : FlexEffect(name) {
}


void EffectSideRainbow::init(Display& display) {
  Effect::init(display);
}


void EffectSideRainbow::process(Display& display) {
  switch (Protogen.getSidePanelType()) {
  case SIDE_PANEL_STRIP:
    process_strip(display);
    break;
  case SIDE_PANEL_CIRCLE:
    process_circle(display);
    break;
  }
}


void EffectSideRainbow::release(Display& display) {
  
  Effect::release(display);
}


void EffectSideRainbow::process_strip(Display& display) {
  const int GROUP = 4;
  const int RAINBOW_TRANSITION = 32;
  const int OFF_SPEED = 20;

  size_t width = display.getWidth();
  size_t numleds = width / 2;

  int off_index = (color_millis() / OFF_SPEED) % numleds;

  for (int i = 0; i < display.getHeight(); i++) {
    for (int j = 0; j < numleds / GROUP; j++) {
      uint8_t r;
      uint8_t g;
      uint8_t b;

      uint16_t hue = (color_millis() / _rainbowSpeed_ms - (j * RAINBOW_TRANSITION)) % 360;
      h2rgb(hue, r, g, b);

      for (int k = 0; k < GROUP; k++) {
        int index = (j * GROUP) + k;
        if ((index - off_index + numleds) % numleds < GROUP) {
          continue;
        }

        display.setPixel(index, i, r, g, b);
        display.setPixel(index + numleds, i, r, g, b);
      }
    }
  }
}


void EffectSideRainbow::process_circle(Display& display) {
  const float ANGLE_MULTIPLIER = 0.5f;
  const float TICK_MULTIPLIER_1 = 0.2f;
  const float TICK_MULTIPLIER_2 = 0.05f;
  const int COLOR_ANGLE = (int)(360 * ANGLE_MULTIPLIER);

  size_t width = display.getWidth();
  size_t numleds = width / 2;
  timer_ms_t tick = color_millis();

  for (int i = 0; i < display.getHeight(); i++) {
    for (int j = 0; j < numleds; j++) {
      uint16_t h1 = (int)((int)(((360.0f - SIDE_PANEL_CIRCLE_DATA[j].first) * ANGLE_MULTIPLIER) + (tick * TICK_MULTIPLIER_1)) % COLOR_ANGLE + (tick * TICK_MULTIPLIER_2)) % 360;
      uint16_t h2 = (int)((int)(((SIDE_PANEL_CIRCLE_DATA[j].first) * ANGLE_MULTIPLIER) + (tick * TICK_MULTIPLIER_1)) % COLOR_ANGLE + (tick * TICK_MULTIPLIER_2)) % 360;

      uint8_t r, g, b;
      h2rgb(h1, r, g, b);

      uint8_t r2, g2, b2;
      h2rgb(h2, r2, g2, b2);

      display.setPixel(j, i, r, g, b);
      display.setPixel(j + numleds, i, r2, g2, b2);
    }
  }
}


};
