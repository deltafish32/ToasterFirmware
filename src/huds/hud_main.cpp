#include "hud_main.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDMain hud_main;


void HUDMain::init() {
  HUDMenu::init();

  clearMenu();
  addMenu("Emotion",   [](HUDBase* p) { p->nextHUD(&hud_emotiondir); });
  addMenu("Bright.",   [](HUDBase* p) { p->nextHUD(&hud_brightness); });
  addMenu("Options",   [](HUDBase* p) { p->nextHUD(&hud_options); });
  addMenu("Color",     [](HUDBase* p) { p->nextHUD(&hud_colors); });
}

};
