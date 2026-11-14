#pragma once
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
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

  virtual void loop() override;

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

  typedef struct _KEY_EVENT {
    uint16_t key;
    uint8_t mode;
  } KEY_EVENT;
  static const UBaseType_t KEY_QUEUE_LENGTH = 16;
  QueueHandle_t _key_queue{nullptr};

protected:
  void setHUD(HUDBase* hud);
  void processKeys();

};

};
