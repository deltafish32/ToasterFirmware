#pragma once
#include "config/configure.h"

#include <Adafruit_SSD1306.h>


namespace toaster {

class HUDBase {
public:
  virtual void init() {
    setStep(0);

    nextHUD(nullptr);
  }

  virtual void process(Adafruit_SSD1306& oled) = 0;
  virtual void release() {
  }

  void setDirty() {
    _dirty = true;
  }

  virtual void pressKey(uint16_t key) {
  }

  HUDBase *getNextHUD() {
    return _next;
  }

  bool isClearStack() {
    return _clearStack;
  }

  bool isPrevRequest() {
    return _prevFlag;
  }

protected:
  int _step{0};
  uint32_t _tick{0};
  bool _dirty{false};
  HUDBase *_next{nullptr};
  bool _clearStack{false};
  bool _prevFlag{false};

  void setStep(int step) {
    _step = step;
    _dirty = true;

    restartTimer();
  }

  bool timeout(uint32_t t) {
    return ((millis() - _tick) >= t);
  }

  void restartTimer() {
    _tick = millis();
  }

public:
  void nextHUD(HUDBase *next, bool clearStack = false) {
    _next = next;
    _clearStack = clearStack;
    _prevFlag = false;
  }

  void prevHUD() {
    _prevFlag = true;
  }
};

};
