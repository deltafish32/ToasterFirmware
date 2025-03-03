#pragma once
#include "hud_menu.h"


namespace toaster {

class HUDShortcut : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key, uint8_t mode);

protected:
  uint16_t _shortcut_key;
  uint8_t _shortcut_count;

protected:
  static uint16_t upperToLower(uint16_t key);

};


class HUDShortcutSelect : public HUDMenu {
public:
  virtual void init();

protected:
  virtual void refreshHighlight();

};


extern HUDShortcut hud_shortcut;
extern HUDShortcutSelect hud_shortcut_select;

};
