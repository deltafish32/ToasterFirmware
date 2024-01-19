#pragma once
#include "hud_menu.h"


namespace toaster {

class HUDEmotions : public HUDMenu {
public:
  HUDEmotions() {
    _pthis = this;
  }
  virtual void init();

public:
  static HUDEmotions* _pthis;
  static void addEmotion(const char* name, const char* emotion) {
    _pthis->_menuData.push_back({1, name, nullptr, emotion});
  }

};

extern HUDEmotions hud_emotions;

};
