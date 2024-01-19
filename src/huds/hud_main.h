#pragma once
#include "hud_menu.h"


namespace toaster {

class HUDMain : public HUDMenu {
public:
  virtual void init();

};

extern HUDMain hud_main;

};
