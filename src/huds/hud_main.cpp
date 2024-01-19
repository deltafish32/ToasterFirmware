#include "hud_main.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDMain hud_main;


void HUDMain::init() {
  HUDMenu::init();

  static HUDMenu::MENU_ITEM menu_main[] = {
    {0, "Emotion",   [](HUDBase* p) { p->nextHUD(&hud_emotions); }},
    {0, "Bright.",   [](HUDBase* p) { p->nextHUD(&hud_brightness); }},
    {0, "Color",     [](HUDBase* p) { p->nextHUD(&hud_colors); }},
  };

  static const int menu_main_count = sizeof(menu_main) / sizeof(menu_main[0]);

  setMenuData(menu_main, menu_main_count);
}

};
