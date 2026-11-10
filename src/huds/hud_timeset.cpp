#include "hud_timeset.h"


#include "huds.h"
#include "hud.h"
#include "protogen.h"


namespace toaster {

HUDTimeset hud_timeset;


void HUDTimeset::init() {
  HUDBase::init();

  _select = 0;
  _hour = Protogen._rtc.getHour();
  _minute = Protogen._rtc.getMinute();
}


void HUDTimeset::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    if (1) {
      oled.clearDisplay();

      int timeout_width = OLED_WIDTH * (TIMEOUT_INACTIVE_MS - getTimeElapsed()) / TIMEOUT_INACTIVE_MS;
      timeout_width = std::min(std::max(timeout_width, 0), OLED_WIDTH);
      oled.drawFastHLine(0, 0, timeout_width, SSD1306_WHITE);

      oled.drawBitmap(0, (OLED_HEIGHT - BITMAP_PREV_HEIGHT) / 2, BITMAP_PREV, BITMAP_PREV_WIDTH, BITMAP_PREV_HEIGHT, SSD1306_WHITE);
      oled.drawBitmap(OLED_WIDTH - BITMAP_OK_WIDTH, (OLED_HEIGHT - BITMAP_OK_HEIGHT) / 2, BITMAP_OK, BITMAP_OK_WIDTH, BITMAP_OK_HEIGHT, SSD1306_WHITE);

      if (_select == 0) {
        oled.drawBitmap(42, 0, BITMAP_UP, BITMAP_UP_WIDTH, BITMAP_UP_HEIGHT, SSD1306_WHITE);
        oled.drawBitmap(42, OLED_HEIGHT - BITMAP_DOWN_HEIGHT, BITMAP_DOWN, BITMAP_DOWN_WIDTH, BITMAP_DOWN_HEIGHT, SSD1306_WHITE);
      }
      else {
        oled.drawBitmap(68, 0, BITMAP_UP, BITMAP_UP_WIDTH, BITMAP_UP_HEIGHT, SSD1306_WHITE);
        oled.drawBitmap(68, OLED_HEIGHT - BITMAP_DOWN_HEIGHT, BITMAP_DOWN, BITMAP_DOWN_WIDTH, BITMAP_DOWN_HEIGHT, SSD1306_WHITE);
      }

      oled.setCursor(42, 24);
      setFont(oled);

      char time_str[16];
      snprintf(time_str, sizeof(time_str), "%02d:%02d", _hour, _minute);
      oled.write(time_str);

      oled.display();
    }
    if (timeout(TIMEOUT_INACTIVE_MS)) {
      nextHUD(&hud_dashboard, true);
      // prevHUD();
    }
    break;
  }
}


void HUDTimeset::release() {
}


void HUDTimeset::pressKey(uint16_t key, uint8_t mode) {
  if ((key == 'f' || key == 'j')
   || mode == KM_KEYCODE && (key == VK_UP)) {
    if (_select == 0) {
      ++_hour;
      if (_hour >= 24) {
        _hour = 0;
      }
    }
    else {
      ++_minute;
      if (_minute >= 60) {
        _minute = 0;
      }
    }
  }

  if ((key == 's' || key == 'l')
   || mode == KM_KEYCODE && (key == VK_DOWN)) {
    if (_select == 0) {
      --_hour;
      if (_hour >= 24) {
        _hour = 23;
      }
    }
    else {
      --_minute;
      if (_minute >= 60) {
        _minute = 59;
      }
    }
  }

  if ((key == 'd' || key == 'k')
   || mode == KM_KEYCODE && (key == VK_RIGHT)) {
    ++_select;
    if (_select > 1) {
      _select = 0;

      Protogen._rtc.setTime(_hour, _minute, 0);

      prevHUD();
    }
  }

  if ((key == 'a' || key == ';')
   || mode == KM_KEYCODE && (key == VK_LEFT)
   || key == 'A' || key == 'S' || key == 'D' || key == 'F'
   || key == 'J' || key == 'K' || key == 'L' || key == ':') {
    prevHUD();
  }

  restartTimer();
}


};
