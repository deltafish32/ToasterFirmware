#pragma once

#include "config/configure.h"
#include "display.h"
#include <NeoPixelBus.h>


namespace toaster {

class NeoPixelDisplay : public Display {
public:
  NeoPixelDisplay();

public:
  virtual bool begin(int numleds, int pin);

public:
  virtual void beginDraw();
  virtual void endDraw();

protected:
  NEOPIXEL_TYPE* _neopixel{nullptr};

  static uint8_t calcBrightness(uint8_t value, uint8_t brightness) {
    return (uint8_t)((uint16_t)value * brightness / 255);
  }

};

};
