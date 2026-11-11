#include "hud_selector.h"


#include "huds.h"
#include "hud.h"
#include "protogen.h"


namespace toaster {

HUDSelector hud_selector;


void HUDSelector::init() {
  HUDBase::init();
}


void HUDSelector::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    if (1) {
      oled.clearDisplay();

      int timeout_width = OLED_WIDTH * (TIMEOUT_INACTIVE_MS - getTimeElapsed()) / TIMEOUT_INACTIVE_MS;
      timeout_width = std::min(std::max(timeout_width, 0), OLED_WIDTH);
      oled.drawFastHLine(0, 0, timeout_width, SSD1306_WHITE);

      oled.drawBitmap(0, (OLED_HEIGHT - BITMAP_PREV_HEIGHT) / 2, BITMAP_PREV, BITMAP_PREV_WIDTH, BITMAP_PREV_HEIGHT, SSD1306_WHITE);
      oled.drawBitmap(OLED_WIDTH - BITMAP_OK_WIDTH, (OLED_HEIGHT - BITMAP_OK_HEIGHT) / 2, BITMAP_OK, BITMAP_OK_WIDTH, BITMAP_OK_HEIGHT, SSD1306_WHITE);

      if (_strings.size() > 1) {
        oled.drawBitmap((OLED_WIDTH - BITMAP_UP_WIDTH) / 2, 0, BITMAP_UP, BITMAP_UP_WIDTH, BITMAP_UP_HEIGHT, SSD1306_WHITE);
        oled.drawBitmap((OLED_WIDTH - BITMAP_DOWN_WIDTH) / 2, OLED_HEIGHT - BITMAP_DOWN_HEIGHT, BITMAP_DOWN, BITMAP_DOWN_WIDTH, BITMAP_DOWN_HEIGHT, SSD1306_WHITE);
      }

      oled.setCursor(12 * 4, 24);
      setFont(oled);

      if (!_strings.empty()) {
        writeCenter(oled, _strings[_select_index].c_str());
      }

      oled.display();
    }
    if (timeout(TIMEOUT_INACTIVE_MS)) {
      nextHUD(&hud_dashboard, true);
      // prevHUD();
    }
    break;
  }
}


void HUDSelector::release() {
}


void HUDSelector::pressKey(uint16_t key, uint8_t mode) {
  if ((key == 'f' || key == 'j')
   || mode == KM_KEYCODE && (key == VK_UP)) {
    --_select_index;
    if (_select_index < 0) {
      // _select_index = 0;
      _select_index = _strings.size() - 1;
    }
  }

  if ((key == 's' || key == 'l')
   || mode == KM_KEYCODE && (key == VK_DOWN)) {
    ++_select_index;
    if (_select_index > _strings.size() - 1) {
      // _select_index = _strings.size() - 1;
      _select_index = 0;
    }
  }

  if ((key == 'd' || key == 'k')
   || mode == KM_KEYCODE && (key == VK_RIGHT)) {
    if (_on_select != nullptr) {
      _on_select(_select_index, _on_select_param);
    }

    prevHUD();
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
