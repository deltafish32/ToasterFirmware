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
    if (_dirty) {
      _dirty = false;
      
      oled.clearDisplay();

      oled.drawBitmap(0, (OLED_HEIGHT - BITMAP_PREV_HEIGHT) / 2, BITMAP_PREV, BITMAP_PREV_WIDTH, BITMAP_PREV_HEIGHT, SSD1306_WHITE);
      oled.drawBitmap((OLED_WIDTH - BITMAP_UP_WIDTH) / 2, 0, BITMAP_UP, BITMAP_UP_WIDTH, BITMAP_UP_HEIGHT, SSD1306_WHITE);
      oled.drawBitmap((OLED_WIDTH - BITMAP_DOWN_WIDTH) / 2, OLED_HEIGHT - BITMAP_DOWN_HEIGHT, BITMAP_DOWN, BITMAP_DOWN_WIDTH, BITMAP_DOWN_HEIGHT, SSD1306_WHITE);

      oled.setCursor(12 * 4, 24);
      setFont(oled);

      if (!_strings.empty()) {
        oled.write(_strings[_select_index].c_str());
      }

      oled.display();
    }
    if (timeout(TIMEOUT_INACTIVE_MS)) {
      prevHUD();
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

    setDirty();
  }

  if ((key == 's' || key == 'l')
   || mode == KM_KEYCODE && (key == VK_DOWN)) {
    ++_select_index;
    if (_select_index > _strings.size() - 1) {
      // _select_index = _strings.size() - 1;
      _select_index = 0;
    }

    setDirty();
  }

  if ((key == 'd' || key == 'k')
   || mode == KM_KEYCODE && (key == VK_RIGHT)) {
    if (_on_select != nullptr) {
      _on_select(_select_index, _on_select_param);
    }

    prevHUD();
  }

  if ((key == 'a' || key == ';')
   || mode == KM_KEYCODE && (key == VK_LEFT)) {
    prevHUD();
  }

  restartTimer();
}


};
