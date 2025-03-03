#include "hud_main.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDMain hud_main;


void HUDMain::init() {
  HUDMenu::init();

  clearMenu();
  addMenu("Emotion",   [](HUDBase* p, const char*) { p->nextHUD(&hud_emotiondir); });
  addMenu("Shortcut",  [](HUDBase* p, const char*) { p->nextHUD(&hud_shortcut_select); });
  addMenu("Options",   [](HUDBase* p, const char*) { p->nextHUD(&hud_options); });
  addMenu("Color",     [](HUDBase* p, const char*) { p->nextHUD(&hud_colors); });
}

};
