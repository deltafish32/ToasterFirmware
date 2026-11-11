#pragma once
#include "hud_base.h"
#include "hal/usb_storage.h"


namespace toaster {

// USB 저장소 모드 화면
// 일반 HUD 루프가 돌지 않는 USB 저장소 모드에서 UsbStorage::run()의 콜백으로 직접 그립니다.
// USB storage mode screen.
// Drawn directly from the UsbStorage::run() callback, since the regular HUD loop does not run in this mode.
class HUDUsbStorage : public HUDBase {
public:
  virtual void process(Adafruit_SSD1306& oled) {
  }

public:
  void draw(Adafruit_SSD1306& oled, const UsbStorage::Status& status);

protected:
  uint32_t _frame{0};

protected:
  void drawAnimation(Adafruit_SSD1306& oled, int16_t x, int16_t y, int16_t w, int16_t h, const UsbStorage::Status& status, bool reading, bool writing);

};

extern HUDUsbStorage hud_usb_storage;

};
