#pragma once
#include <vector>

#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1

#include "huds/hud_base.h"
#include "lib/worker.h"


namespace toaster {

class HeadUpDisplay : public Worker {
public:
  HeadUpDisplay();

  bool begin(uint8_t i2c_addr, uint32_t target_fps);
  void pressKey(uint16_t key);

public:
  Adafruit_SSD1306 _oled;

protected:
  void work();

protected:
  bool _init{false};
  HUDBase* _hud{nullptr};
  std::vector<HUDBase*> _hudStack;

protected:
  void setHUD(HUDBase* hud);

};

};
