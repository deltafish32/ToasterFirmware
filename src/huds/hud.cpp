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

  Worker::begin(target_fps);

  _init = true;

  return true;
}


void HeadUpDisplay::pressKey(uint16_t key) {
  if (_init == false) {
    return;
  }

  if (_hud != nullptr) {
    Protogen.syncLock();

    _hud->pressKey(key);
    
    Protogen.syncUnlock();
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
