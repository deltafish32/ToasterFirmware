#pragma once

#include "config/configure.h"
#include "display.h"
#include <NeoPixelBus.h>


#ifdef CONFIG_IDF_TARGET_ESP32S3
// #define NEOPIXEL_TYPE                            NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>
#define NEOPIXEL_TYPE                            NeoPixelBus<NeoGrbFeature, NeoEsp32BitBangWs2812xMethod>
#else
// 주의: I2S 버스 사용시 HUB75와 충돌합니다.
#define NEOPIXEL_TYPE                            NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod>
#endif



namespace toaster {

class NeoPixelDisplay : public Display {
public:
  NeoPixelDisplay();

public:
  virtual bool begin(int numleds, int pin, bool reversed);

public:
  virtual void beginDraw();
  virtual void endDraw();

protected:
  NEOPIXEL_TYPE* _neopixel{nullptr};
  bool _reversed{false};
  uint8_t* _prev_buffer{nullptr};

  static uint8_t calcBrightness(uint8_t value, uint8_t brightness) {
    return (uint8_t)((uint16_t)value * brightness / 255);
  }

};

};
