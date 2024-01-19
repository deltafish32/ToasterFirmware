#include "hud_menu.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

void HUDMenu::init() {
  HUDBase::init();

  // _menuIndex = 0;
  // _menuScroll = 0;
}

void HUDMenu::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    if (_dirty) {
      _dirty = false;

      drawMenu(oled);
      oled.display();
    }
    if (timeout(3000)) {
      nextHUD(&hud_dashboard, true);
    }
    break;
  }
}

void HUDMenu::release() {
}

void HUDMenu::pressKey(uint16_t key) {
  if (key == 'H') {
    prevHUD();
  }
  if (key == 'J' || key == 'S') {
    if (_menuData.empty() == false) {
      if (_menuData.size() > MENU_LINES) {
        // Scroll
        _menuIndex += 1;
        if (_menuIndex > MENU_LINES - 1) {
          _menuIndex = MENU_LINES - 1;

          _menuScroll += 1;
          if (_menuScroll > _menuData.size() - _menuIndex - 1) {
            if (_menuLoop) {
              _menuIndex = 0;
              _menuScroll = 0;
            }
            else {
              _menuScroll = _menuData.size() - _menuIndex - 1;
            }
          }
        }
      }
      else {
        // No scroll
        _menuIndex += 1;
        if (_menuIndex > _menuData.size() - 1) {
          if (_menuLoop) {
            _menuIndex = 0;
          }
          else {
            _menuIndex = _menuData.size() - 1;
          }
        }
      }

      setDirty();
      restartTimer();
    }
  }
  if (key == 'K' || key == 'F') {
    if (_menuData.empty() == false) {
      _menuIndex -= 1;
      if (_menuIndex < 0) {
        _menuIndex = 0;

        _menuScroll -= 1;
        if (_menuScroll < 0) {
          if (_menuLoop) {
            if (_menuData.size() > MENU_LINES) {
              // Scroll
              _menuIndex = MENU_LINES - 1;
              _menuScroll = _menuData.size() - _menuIndex - 1;
            }
            else {
              // No Scroll
              _menuIndex = _menuData.size() - 1;
              _menuScroll = 0;
            }
          }
          else {
            _menuScroll = 0;
          }
        }
      }

      setDirty();
      restartTimer();
    }
  }
  if (key == 'L' || key == 'D') {
    int sel_line = _menuIndex + _menuScroll;

    if (_menuData[sel_line].type == 0) {
      _menuData[sel_line].func(this);
    }
    else {
      Protogen.setEmotion(_menuData[sel_line].emotion.c_str());
    }
    restartTimer();
  }
}


void HUDMenu::setMenuData(const MENU_ITEM* data, int count) {
  // _menuData = data;
  // _menuItemCount = count;

  _menuData.clear();
  for (int i = 0; i < count; i++) {
    _menuData.push_back(data[i]);
  }
}


void HUDMenu::drawMenu(Adafruit_SSD1306& oled) {
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(2);
  oled.setCursor(0, 0);

  if (_menuData.empty()) {
    return;
  }

  int sel_line = _menuIndex + _menuScroll;
  for (int i = 0; i < MENU_LINES; i++) {
    int cur_line = _menuScroll + i;

    oled.write((sel_line == cur_line) ? '>' : ' ');

    if (cur_line >= _menuData.size()) break;

    oled.write(_menuData[cur_line].text.c_str());
    oled.write('\n');
  }

}

};
