#pragma once
#include "hud_base.h"
#include <vector>


namespace toaster {

class HUDMenu : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key);

public:
  typedef struct _MENU_ITEM {
    uint8_t type;
    std::string text;
    std::function<void(HUDBase*)> func;
    std::string emotion;
  } MENU_ITEM;

protected:
  enum {
    MENU_LINES = 4,
  };

protected:
  int _menuIndex{0};
  int _menuScroll{0};
  // const MENU_ITEM* _menuData{nullptr};
  // int _menuItemCount{0};
  std::vector<MENU_ITEM> _menuData;
  bool _menuLoop{true};

protected:
  void setMenuData(const MENU_ITEM* data, int count);
  void drawMenu(Adafruit_SSD1306& oled);

};

};
