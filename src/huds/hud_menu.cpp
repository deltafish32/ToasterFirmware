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
    if (timeout(TIMEOUT_INACTIVE)) {
      nextHUD(&hud_dashboard, true);
      // prevHUD();
    }
    break;
  }
}

void HUDMenu::release() {
}

void HUDMenu::pressKey(uint16_t key, uint8_t mode) {
  if ((key == 's' || key == 'l')
   || mode == KM_KEYCODE && (key == VK_DOWN)) {
    size_t menuCount = getMenuCount();
    
    if (_menuData.empty() == false) {
      if (menuCount > MENU_LINES) {
        // Scroll
        _menuIndex += 1;
        if (_menuIndex > MENU_LINES - 1) {
          _menuIndex = MENU_LINES - 1;

          _menuScroll += 1;
          if (_menuScroll > menuCount - _menuIndex - 1) {
            if (_menuLoop) {
              _menuIndex = 0;
              _menuScroll = 0;
            }
            else {
              _menuScroll = menuCount - _menuIndex - 1;
            }
          }
        }
      }
      else {
        // No scroll
        _menuIndex += 1;
        if (_menuIndex > menuCount - 1) {
          if (_menuLoop) {
            _menuIndex = 0;
          }
          else {
            _menuIndex = menuCount - 1;
          }
        }
      }

      setDirty();
      restartTimer();
    }
  }
  else if ((key == 'f' || key == 'j')
        || mode == KM_KEYCODE && (key == VK_UP)) {
    size_t menuCount = getMenuCount();

    if (_menuData.empty() == false) {
      _menuIndex -= 1;
      if (_menuIndex < 0) {
        _menuIndex = 0;

        _menuScroll -= 1;
        if (_menuScroll < 0) {
          if (_menuLoop) {
            if (menuCount > MENU_LINES) {
              // Scroll
              _menuIndex = MENU_LINES - 1;
              _menuScroll = menuCount - _menuIndex - 1;
            }
            else {
              // No Scroll
              _menuIndex = menuCount - 1;
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
  else if ((key == 'd' || key == 'k')
        || mode == KM_KEYCODE && (key == VK_RIGHT)) {
    int sel_line = _menuIndex + _menuScroll;
    size_t menuCount = getMenuCount();
    bool highlight = false;

    if (_use_backbutton && sel_line == menuCount - 1) { // back buttons
      prevHUD();
    }
    else {
      const auto& menu = _menuData[sel_line];

      switch (menu.type) {
      case MENU_FUNC:
        menu.func(this);
        highlight = true;
        break;
      case MENU_EMOTION:
        Protogen.setEmotion(menu.emotion.c_str());
        highlight = true;
        break;
      case MENU_HUD:
        nextHUD(menu.hud);
        break;
      }
    }

    if (highlight) {
      _menuHighlight = sel_line;
      _menuHasHighlight = true;
      setDirty();
    }

    restartTimer();
  }
  else {
    prevHUD();
  }
}


void HUDMenu::drawMenu(Adafruit_SSD1306& oled) {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(0, 0);

  int sel_line = _menuIndex + _menuScroll;
  for (int i = 0; i < MENU_LINES; i++) {
    int cur_line = _menuScroll + i;

    if (_menu_use_highlight && _menuHasHighlight && cur_line == _menuHighlight) {
      oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    }
    else {
      oled.setTextColor(SSD1306_WHITE);
    }

    oled.write((sel_line == cur_line) ? '>' : ' ');

    if (cur_line < _menuData.size()) {
      const size_t MENU_MAX_LEN = 9;
      char menu[MENU_MAX_LEN + 1] = {0, };
      strncpy(menu, _menuData[cur_line].text.c_str(), MENU_MAX_LEN);
      oled.write(menu);

      oled.write('\n');
    }
    else if (_use_backbutton && cur_line == _menuData.size()) {
      oled.write("<-\n");
    }
  }

}

};
