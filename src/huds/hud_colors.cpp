#include "hud_colors.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDColors hud_colors;


void HUDColors::init() {
  HUDMenu::init();

  clearMenu();
  addMenu("Personal",   [](HUDBase* p, const char*) { Protogen.setColorMode(PCM_PERSONAL); });
  addMenu("Original",   [](HUDBase* p, const char*) { Protogen.setColorMode(PCM_ORIGINAL); });
  addMenu("R. single",  [](HUDBase* p, const char*) { Protogen.setColorMode(PCM_RAINBOW_SINGLE); });
  addMenu("Rainbow",    [](HUDBase* p, const char*) { Protogen.setColorMode(PCM_RAINBOW); });
  addMenu("Gradation",  [](HUDBase* p, const char*) { Protogen.setColorMode(PCM_GRADATION); });

  _menu_use_highlight = _menuHasHighlight = true;
  _menuHighlight = Protogen.getColorMode();
  
}

};
