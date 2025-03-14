#include "hud_options.h"


#include "huds.h"
#include "protogen.h"

#include "lib/logger.h"


namespace toaster {

HUDOptions hud_options;


void HUDOptions::init() {
  HUDMenu::init();

  clearMenu();
  addMenu("Bright.",   [](HUDBase* p, const char*) { p->nextHUD(&hud_brightness); });
  addMenu("Boop",      [](HUDBase* p, const char*) { 
    std::vector<std::string> list = {"off", "on"};
    hud_selector.selectString(list, Protogen._boopsensor.getEnabled() ? 1 : 0, [](int index, void*) {
      Protogen._boopsensor.setEnabled(index == 0 ? false : true);
    });
    p->nextHUD(&hud_selector);
  });
}

};
