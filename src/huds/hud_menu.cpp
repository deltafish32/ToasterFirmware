#include "hud_menu.h"


#include "huds.h"
#include "protogen.h"
#include "lib/random.h"


namespace toaster {

void HUDMenu::init() {
  HUDBase::init();

  // _menuIndex = 0;
  // _menuScroll = 0;
}


void HUDMenu::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    if (1) {
      drawMenu(oled);

      int timeout_width = OLED_WIDTH * (TIMEOUT_INACTIVE_MS - getTimeElapsed()) / TIMEOUT_INACTIVE_MS;
      timeout_width = std::min(std::max(timeout_width, 0), OLED_WIDTH);
      oled.drawFastHLine(0, 0, timeout_width, SSD1306_WHITE);

      oled.display();
    }

    if (timeout(TIMEOUT_INACTIVE_MS)) {
      nextHUD(&hud_dashboard, true);
      // prevHUD();
    }
    break;
  }
}


void HUDMenu::release() {
}


void HUDMenu::pressKey(uint16_t key, uint8_t mode) {
  Protogen.syncLock();
  
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

    if (_use_backbutton && sel_line == getBackbuttonIndex()) {
      prevHUD();
    }
    else if (_use_shuffle && sel_line == getShuffleIndex()) {
      shuffle();
    }
    else {
      highlight = selectMenu(sel_line);
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

  Protogen.syncUnlock();
}


void HUDMenu::addMenu(const char* text, std::function<void(HUDBase*, const char*)> func, const std::string& param) {
  _menuData.push_back({MENU_FUNC, text, func, param,});
}


void HUDMenu::addEmotion(const char* text, const char* emotion) {
  _menuData.push_back({MENU_FUNC, text, [](HUDBase*, const char* param) { Protogen.setEmotion(param); }, emotion,});
}


void HUDMenu::addHUD(const char* text, HUDBase* hud) {
  _menuData.push_back({MENU_HUD, text, nullptr, "", hud,});
}


void HUDMenu::drawMenu(Adafruit_SSD1306& oled) {
  const int LINE_H = 16;

  oled.clearDisplay();
  oled.setCursor(0, 0);
  setFont(oled);

  int sel_line = _menuIndex + _menuScroll;
  for (int i = 0; i < MENU_LINES; i++) {
    int cur_line = _menuScroll + i;

    if (_menu_use_highlight && _menuHasHighlight && cur_line == _menuHighlight) {
      oled.setTextColor(SSD1306_BLACK);
      oled.fillRect(12, i * LINE_H + 1, oled.width() - 12, LINE_H - 2, SSD1306_WHITE);
    }
    else {
      oled.setTextColor(SSD1306_WHITE);
    }

    writeSpecial(oled, (sel_line == cur_line) ? BITMAP_CURSOR : nullptr);

    if (cur_line < _menuData.size()) {
      oled.write(_menuData[cur_line].text.c_str());
      oled.write('\n');
    }
    else if (_use_backbutton && cur_line == getBackbuttonIndex()) {
      writeSpecial(oled, BITMAP_BACK);
      oled.write("\n");
    }
    else if (_use_shuffle && cur_line == getShuffleIndex()) {
      writeSpecial(oled, BITMAP_SHUFFLE);
      oled.write("\n");
    }
  }

}


bool HUDMenu::selectMenu(int index) {
  const auto& menu = _menuData[index];

  switch (menu.type) {
  case MENU_FUNC:
    menu.func(this, menu.param.c_str());
    return true;
    break;
  case MENU_HUD:
    nextHUD(menu.hud);
    break;
  }

  return false;
}


void HUDMenu::refreshHighlight() {
  setDirty();
}


void HUDMenu::shuffle() {
  std::vector<int> deck;
  deck.reserve(_menuData.size());

  for (int i = 0; i < _menuData.size(); i++) {
    if (_menuHasHighlight && i == _menuHighlight) {
      continue;
    }

    deck.push_back(i);
  }

  Random rnd;
  int n = rnd.random(deck.size());

  selectMenu(deck[n]);
  refreshHighlight();
}

};
