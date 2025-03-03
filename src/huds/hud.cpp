#include "hud.h"


#include "huds/huds.h"
#include "protogen.h"


namespace toaster {

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

  _target_fps = target_fps;
  Worker::begin(_target_fps);

  _init = true;

  return true;
}


void HeadUpDisplay::pressKey(uint16_t key, uint8_t mode) {
  if (_init == false) {
    return;
  }

  if (_hud != nullptr) {
    _hud->pressKey(key, mode);
  }
}


void HeadUpDisplay::setPF(timer_pf_t pf) {
  if (pf.type == PF_FREQUENCY) {
    uint32_t hud_freq = pf.frequency;
    while (hud_freq > _target_fps) {
      hud_freq /= 2;
    }
    setFPS(hud_freq);
  }
  else if (pf.type == PF_PERIOD) {
    uint32_t hud_period_ms = pf.period_ms;
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
