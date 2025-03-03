#pragma once
#include "hud_base.h"
#include <vector>


namespace toaster {

class HUDMenu : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key, uint8_t mode);

public:
  enum {
    MENU_FUNC = 0,
    MENU_HUD,
  };

  typedef struct _MENU_ITEM {
    uint8_t type;
    std::string text;
    std::function<void(HUDBase*, const char*)> func;
    std::string param;
    HUDBase* hud;
  } MENU_ITEM;

protected:
  enum {
    MENU_LINES = 4,
  };

protected:
  int _menuIndex{0};
  int _menuScroll{0};
  int _menuHighlight{0};
  bool _menuHasHighlight{false};
  std::vector<MENU_ITEM> _menuData;
  bool _menuLoop{true};

protected:
  bool _menu_use_highlight{false};
  bool _use_shuffle{false};
  bool _use_backbutton{true};

public:
  void clearMenu() {
    _menuData.clear();
  }

  void addMenu(const char* text, std::function<void(HUDBase*, const char*)> func, const std::string& param = "");
  void addEmotion(const char* text, const char* emotion);
  void addHUD(const char* text, HUDBase* hud);

protected:
  void drawMenu(Adafruit_SSD1306& oled);
  size_t getMenuCount() const {
    return _menuData.size() + (_use_backbutton ? 1 : 0) + (_use_shuffle ? 1 : 0);
  }

  int getBackbuttonIndex() const {
    return getMenuCount() - 1;
  }
  int getShuffleIndex() const {
    return getMenuCount() - 1 - (_use_backbutton ? 1 : 0);
  }

  virtual bool selectMenu(int index);
  virtual void refreshHighlight();
  virtual void shuffle();

};

};
