#pragma once
#include "hud_base.h"


namespace toaster {

class HUDShortcut : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key, uint8_t mode);

};

extern HUDShortcut hud_shortcut;

};
