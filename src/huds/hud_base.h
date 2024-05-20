#pragma once
#include "config/configure.h"
#include "lib/timer.h"
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

  virtual void pressKey(uint16_t key, uint8_t mode) {
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
  timer_ms_t _tick_ms{0};
  bool _dirty{false};
  HUDBase *_next{nullptr};
  bool _clearStack{false};
  bool _prevFlag{false};

  void restartTimer() {
    _tick_ms = Timer::get_millis();
  }

  void setStep(int step) {
    _step = step;
    _dirty = true;

    restartTimer();
  }

  bool timeout(timer_ms_t time_ms) {
    return ((Timer::get_millis() - _tick_ms) >= time_ms);
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

protected:
  static const timer_ms_t TIMEOUT_INACTIVE;
};


enum {
  KM_ASCII = 0,
  KM_KEYCODE,

  KM_MAX,
};


enum {
  VK_LEFT  = 0x25,
  VK_UP    = 0x26,
  VK_RIGHT = 0x27,
  VK_DOWN  = 0x28,
};


extern const size_t BITMAP_PREV_WIDTH;
extern const size_t BITMAP_PREV_HEIGHT;
extern const uint8_t PROGMEM BITMAP_PREV[];

extern const size_t BITMAP_UP_WIDTH;
extern const size_t BITMAP_UP_HEIGHT;
extern const uint8_t PROGMEM BITMAP_UP[];

extern const size_t BITMAP_DOWN_WIDTH;
extern const size_t BITMAP_DOWN_HEIGHT;
extern const uint8_t PROGMEM BITMAP_DOWN[];

extern const size_t BITMAP_CELSIUS_WIDTH;
extern const size_t BITMAP_CELSIUS_HEIGHT;
extern const uint8_t PROGMEM BITMAP_CELSIUS[];

extern const size_t BITMAP_EX_MARK_WIDTH;
extern const size_t BITMAP_EX_MARK_HEIGHT;
extern const uint8_t PROGMEM BITMAP_EX_MARK[];

extern const size_t BITMAP_LOCK_WIDTH;
extern const size_t BITMAP_LOCK_HEIGHT;
extern const uint8_t PROGMEM BITMAP_LOCK[];

extern const size_t BITMAP_PLAY_WIDTH;
extern const size_t BITMAP_PLAY_HEIGHT;
extern const uint8_t PROGMEM BITMAP_PLAY[3][32];

extern const size_t BITMAP_PAUSE_WIDTH;
extern const size_t BITMAP_PAUSE_HEIGHT;
extern const uint8_t PROGMEM BITMAP_PAUSE[];


};
