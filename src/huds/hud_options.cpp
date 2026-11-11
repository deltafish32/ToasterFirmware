#include "hud_options.h"


#include "huds.h"
#include "protogen.h"
#include "hal/usb_storage.h"

#include "lib/logger.h"


namespace toaster {

HUDOptions hud_options;


void HUDOptions::init() {
  HUDMenu::init();

  clearMenu();
  addMenu("Bright.",   [](HUDBase* p, const char*) { p->nextHUD(&hud_brightness); });

  if (Protogen.isLightSensorUse()) {
    addMenu("Auto B.",   [](HUDBase* p, const char*) { 
      std::vector<std::string> list = {"off", "on"};
      hud_selector.selectString(list, Protogen.getAutoBrightness() ? 1 : 0, [](int index, void*) {
        Protogen.setAutoBrightness(index != 0);
      });
      p->nextHUD(&hud_selector);
    });
  }
  addMenu("Boop",      [](HUDBase* p, const char*) { 
    std::vector<std::string> list = {"off", "on"};
    hud_selector.selectString(list, Protogen._boopsensor.getEnabled() ? 1 : 0, [](int index, void*) {
      Protogen._boopsensor.setEnabled(index == 0 ? false : true);
    });
    p->nextHUD(&hud_selector);
  });

  if (Protogen._rtc.isBegin()) {
    addMenu("Time",      [](HUDBase* p, const char*) { 
      p->nextHUD(&hud_timeset);
    });
  }

  if (UsbStorage::isSupported()) {
    addMenu("USB",       [](HUDBase* p, const char*) {
      std::vector<std::string> list = {"confirm"};
      hud_selector.selectString(list, 0, [](int, void*) {
        Protogen.requestUsbStorage();
      });
      p->nextHUD(&hud_selector);
    });
  }
}

};
