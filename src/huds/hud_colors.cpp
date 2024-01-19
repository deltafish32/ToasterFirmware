#include "hud_colors.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDColors hud_colors;


void HUDColors::init() {
  HUDMenu::init();

  static HUDMenu::MENU_ITEM menu_color[] = {
    {0, "Original",   [](HUDBase* p) { Protogen.setColorMode(PCM_ORIGINAL); }},
    {0, "Personal",   [](HUDBase* p) { Protogen.setColorMode(PCM_PERSONAL); }},
    {0, "R. single",  [](HUDBase* p) { Protogen.setColorMode(PCM_RAINBOW_SINGLE); }},
    {0, "Rainbow",    [](HUDBase* p) { Protogen.setColorMode(PCM_RAINBOW); }},
    {0, "Gradation",  [](HUDBase* p) { Protogen.setColorMode(PCM_GRADATION); }},
  };

  static const int menu_color_count = sizeof(menu_color) / sizeof(menu_color[0]);

  setMenuData(menu_color, menu_color_count);
}

};
