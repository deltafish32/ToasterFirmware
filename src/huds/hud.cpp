#include "hud.h"


#include "huds/huds.h"
#include "protogen.h"
#include "lib/logger.h"


namespace toaster {

static const char* TAG = "HUD";

HeadUpDisplay::HeadUpDisplay() : _oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET) {
}


bool HeadUpDisplay::begin(uint8_t i2c_addr, uint32_t target_fps) {
  if (_init) {
    return true;
  }

  if (!_oled.begin(SSD1306_SWITCHCAPVCC, i2c_addr)) {
    return false;
  }

  _oled.clearDisplay();
  _oled.display();

  setHUD(&hud_splash);

  _target_fps = (target_fps > 0) ? target_fps : 1;
  Worker::begin(_target_fps);

  if (_key_queue == nullptr) {
    _key_queue = xQueueCreate(KEY_QUEUE_LENGTH, sizeof(KEY_EVENT));
    if (_key_queue == nullptr) {
      TF_LOGE(TAG, "key queue creation failed");
    }
  }

  _init = true;

  return true;
}


void HeadUpDisplay::pressKey(uint16_t key, uint8_t mode) {
  if (_init == false || _key_queue == nullptr) {
    return;
  }

  KEY_EVENT event = {key, mode};
  if (xQueueSend(_key_queue, &event, 0) != pdTRUE) {
    TF_LOGW(TAG, "key queue full, key 0x%04x dropped", key);
  }
}


void HeadUpDisplay::loop() {
  processKeys();
  Worker::loop();
}


void HeadUpDisplay::processKeys() {
  if (_init == false || _key_queue == nullptr) {
    return;
  }

  KEY_EVENT event;
  while (xQueueReceive(_key_queue, &event, 0) == pdTRUE) {
    if (_hud != nullptr) {
      _hud->pressKey(event.key, event.mode);
    }
  }
}


void HeadUpDisplay::setPF(timer_pf_t pf) {
  if (pf.type == PF_FREQUENCY) {
    uint32_t hud_freq = (pf.frequency > 0) ? pf.frequency : 1;
    while (hud_freq > _target_fps && hud_freq > 1) {
      hud_freq /= 2;
    }
    setFPS(hud_freq);
  }
  else if (pf.type == PF_PERIOD) {
    uint32_t hud_period_ms = (pf.period_ms > 0) ? pf.period_ms : 1;
    while (hud_period_ms < (1000 / _target_fps)) {
      hud_period_ms *= 2;
    }
    setPeriod(hud_period_ms);
  }
}


bool HeadUpDisplay::work() {
  if (_init == false || _hud == nullptr) {
    return false;
  }

  _hud->process(_oled);

  if (_hud->isPrevRequest() &&_hudStack.empty() == false) {
    setHUD(_hudStack.back());
    _hudStack.pop_back();
  }
  else {
    auto next = _hud->getNextHUD();
    if (next != nullptr) {
      if (_hud->isClearStack()) {
        _hudStack.clear();
      }
      else {
        _hudStack.push_back(_hud);
      }
      
      setHUD(next);
    }
  }

  return true;
}


void HeadUpDisplay::setHUD(HUDBase* hud) {
  if (_hud != hud) {
    if (_hud != nullptr) {
      _hud->release();
    }

    _hud = hud;

    if (_hud != nullptr) {
      _hud->init();
    }
  }

}

};
