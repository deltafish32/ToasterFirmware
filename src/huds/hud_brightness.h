#pragma once
#include "hud_base.h"


namespace toaster {

class HUDBrightness : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key);

protected:
  int getBrightness();
};

extern HUDBrightness hud_brightness;

};
