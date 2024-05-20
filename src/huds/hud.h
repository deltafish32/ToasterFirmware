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

  virtual bool begin(uint8_t i2c_addr, uint32_t target_fps);
  virtual void pressKey(uint16_t key, uint8_t mode = KM_ASCII);

public:
  Adafruit_SSD1306 _oled;

public:
  virtual void setDithering(bool dithering) {
    _dithering = dithering;
  }

  virtual bool getDithering() const {
    return _dithering;
  }

  virtual void setPF(timer_pf_t pf);

protected:
  virtual bool work();

protected:
  bool _init{false};
  HUDBase* _hud{nullptr};
  std::vector<HUDBase*> _hudStack;
  bool _dithering{false};
  uint32_t _target_fps{30};

protected:
  void setHUD(HUDBase* hud);

};

};
