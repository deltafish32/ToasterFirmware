#pragma once
#include "hud_menu.h"


namespace toaster {

class HUDColors : public HUDMenu {
public:
  virtual void init();

};

extern HUDColors hud_colors;

};
