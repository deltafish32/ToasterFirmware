#pragma once
#include "hud_menu.h"


namespace toaster {

class HUDOptions : public HUDMenu {
public:
  virtual void init();

};

extern HUDOptions hud_options;

};
