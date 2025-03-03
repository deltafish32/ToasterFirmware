#include "hud_shortcut.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDShortcut hud_shortcut;
HUDShortcutSelect hud_shortcut_select;


void HUDShortcut::init() {
  HUDBase::init();

  _shortcut_key = 0;
  _shortcut_count = 0;
}


void HUDShortcut::process(Adafruit_SSD1306& oled) {
  static const timer_ms_t TIMEOUT_INPUT_MS = 350;

  oled.clearDisplay();
  setFont(oled);

  oled.setCursor(0, 0);
  writeCenter(oled, "Emotion");

  oled.setCursor(0, 16);
  writeCenter(oled, "shortcut");

  oled.drawFastVLine( 10, 44, 20, SSD1306_WHITE);
  oled.drawFastVLine( 27, 44, 20, SSD1306_WHITE);
  oled.drawFastVLine( 44, 44, 20, SSD1306_WHITE);
  oled.drawFastVLine( 61, 37, 27, SSD1306_WHITE);
  oled.drawFastVLine( 66, 37, 27, SSD1306_WHITE);
  oled.drawFastVLine( 83, 44, 20, SSD1306_WHITE);
  oled.drawFastVLine(100, 44, 20, SSD1306_WHITE);
  oled.drawFastVLine(117, 44, 20, SSD1306_WHITE);

  auto draw_gauge = [&oled](int16_t x, uint8_t count) {
    for (int i = 0; i < count; i++) {
      oled.fillRect(x, 58 - (i * 7), 14, 6, SSD1306_WHITE);
    }
  };

  switch (_shortcut_key) {
  case 's': draw_gauge( 12, _shortcut_count + 1); break;
  case 'd': draw_gauge( 29, _shortcut_count + 1); break;
  case 'f': draw_gauge( 46, _shortcut_count + 1); break;
  case 'j': draw_gauge( 68, _shortcut_count + 1); break;
  case 'k': draw_gauge( 85, _shortcut_count + 1); break;
  case 'l': draw_gauge(102, _shortcut_count + 1); break;
  }

  timer_ms_t timeout_time = (_shortcut_key == 0) ? TIMEOUT_INACTIVE_MS : TIMEOUT_INPUT_MS;
  int timeout_width = OLED_WIDTH * (timeout_time - getTimeElapsed()) / timeout_time;
  timeout_width = std::min(std::max(timeout_width, 0), OLED_WIDTH);
  oled.drawFastHLine(0, 0, timeout_width, SSD1306_WHITE);

  oled.display();

  if (_shortcut_key != 0 && (_shortcut_count >= SHORTCUT_CLICK_COUNT_MAX || timeout(TIMEOUT_INPUT_MS))) {
    uint8_t click_count = std::min(_shortcut_count, (uint8_t)SHORTCUT_CLICK_COUNT_MAX);

    Protogen.syncLock();

    switch (_shortcut_key) {
    case 'f': Protogen.setEmotionShortcut(0, 0, click_count); break;
    case 'd': Protogen.setEmotionShortcut(0, 1, click_count); break;
    case 's': Protogen.setEmotionShortcut(0, 2, click_count); break;
    case 'a': break;
    case 'j': Protogen.setEmotionShortcut(1, 0, click_count); break;
    case 'k': Protogen.setEmotionShortcut(1, 1, click_count); break;
    case 'l': Protogen.setEmotionShortcut(1, 2, click_count); break;
    case ';': break;
    }
    
    Protogen.syncUnlock();

    nextHUD(&hud_dashboard, true);
  }

  if (timeout(TIMEOUT_INACTIVE_MS)) {
    nextHUD(&hud_dashboard, true);
  }
}


void HUDShortcut::release() {
}


void HUDShortcut::pressKey(uint16_t key, uint8_t mode) {
  // ignore initial inputs
  if (!timeout(100)) {
    return;
  }

  char lower_key = upperToLower(key);
  if (lower_key != 0) {
    _shortcut_key = lower_key;
    _shortcut_count = SHORTCUT_CLICK_COUNT_MAX;
  }
  else {
    if (_shortcut_key != key) {
      _shortcut_key = key;
      _shortcut_count = 0;
    }
    else {
      ++_shortcut_count %= SHORTCUT_CLICK_COUNT_MAX;
    }

    _dirty = true;
    restartTimer();
    return;
  }

  nextHUD(&hud_dashboard, true);
}


uint16_t HUDShortcut::upperToLower(uint16_t key) {
  switch (key) {
  case 'F': return 'f';
  case 'D': return 'd';
  case 'S': return 's';
  case 'A': return 'a';
  case 'J': return 'j';
  case 'K': return 'k';
  case 'L': return 'l';
  case ':': return ';';
  }

  return 0;
}


void HUDShortcutSelect::init() {
  HUDMenu::init();

  _menu_use_highlight = true;

  clearMenu();
  for (const auto& it : Protogen.getShortcutList()) {
    addMenu(it.c_str(), [](HUDBase*, const char* param) { Protogen.loadShortcut(param); }, it);
  }

  refreshHighlight();
}


void HUDShortcutSelect::refreshHighlight() {
  _menuHasHighlight = false;

  for (size_t i = 0; i < _menuData.size(); i++) {
    if (strcmp(_menuData[i].text.c_str(), Protogen.getShortcut()) == 0) {
      _menuHighlight = i;
      _menuHasHighlight = true;
      break;
    }
  }

  HUDMenu::refreshHighlight();
}


};
