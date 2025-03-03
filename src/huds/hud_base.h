#pragma once
#include "config/configure.h"
#include "lib/timer.h"
#include <Adafruit_SSD1306.h>
#include "hud_font.h"


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

  bool timeout(timer_ms_t time_ms) const {
    return ((Timer::get_millis() - _tick_ms) >= time_ms);
  }

  timer_ms_t getTimeElapsed() const {
    return (Timer::get_millis() - _tick_ms);
  }
  
  static void writeSpecial(Adafruit_SSD1306& oled, const uint8_t* bitmap) {
    const int16_t BITMAP_W = 12;
    const int16_t BITMAP_H = 16;

    if (bitmap != nullptr) {
      oled.drawBitmap(oled.getCursorX(), oled.getCursorY(), bitmap, BITMAP_W, BITMAP_H, SSD1306_WHITE);
    }
    oled.setCursor(oled.getCursorX() + BITMAP_W, oled.getCursorY());
  }

  static void writeCenter(Adafruit_SSD1306& oled, const char* str) {
    int16_t x, y;
    uint16_t w, h;
    oled.getTextBounds(str, 0, 0, &x, &y, &w, &h);
    oled.setCursor((oled.width() - w) / 2, oled.getCursorY());
    oled.write(str);
  }

  void setFont(Adafruit_SSD1306& oled) {
    oled.setTextSize(1);
    oled.setFont(&Perfect_DOS_VGA_437_Win8pt7b);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);
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
  static const timer_ms_t TIMEOUT_INACTIVE_MS;
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

extern const uint8_t PROGMEM BITMAP_CELSIUS[];
extern const uint8_t PROGMEM BITMAP_EX_MARK[];
extern const uint8_t PROGMEM BITMAP_LOCK[];
extern const uint8_t PROGMEM BITMAP_PLAY[3][32];
extern const uint8_t PROGMEM BITMAP_PAUSE[];
extern const uint8_t PROGMEM BITMAP_SHUFFLE[];
extern const uint8_t PROGMEM BITMAP_BACK[];
extern const uint8_t PROGMEM BITMAP_CURSOR[];

};
