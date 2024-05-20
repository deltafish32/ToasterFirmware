#pragma once
#include "hud_base.h"
#include "hal/display/display.h"


namespace toaster {

class HUDSplash : public HUDBase, protected Display {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key, uint8_t mode);

protected:
  static bool draw_oled_png(Adafruit_SSD1306& oled, const Image* image, int offset_x, int offset_y, uint8_t th = 128);

protected:
  Image* _splash{nullptr};
};

extern HUDSplash hud_splash;

};
