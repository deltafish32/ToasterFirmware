#pragma once
#include "hud_base.h"


namespace toaster {

class HUDDashboard : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key);

protected:
  uint16_t _unlock_key{0};
  uint8_t _unlock_count{0};
  uint32_t _unlock_tick{0};

protected:
  static void mirror(Adafruit_SSD1306& oled, int offset_x, int offset_y);
};

extern HUDDashboard hud_dashboard;

};
