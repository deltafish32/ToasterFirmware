#include "hud_shortcut.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDShortcut hud_shortcut;


void HUDShortcut::init() {
  HUDBase::init();

}


void HUDShortcut::process(Adafruit_SSD1306& oled) {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2);

  oled.setCursor(0, 16);
  oled.write(" Emotion");

  oled.setCursor(0, 32);
  oled.write(" shortcut");

  oled.display();
  
  if (timeout(TIMEOUT_INACTIVE)) {
    nextHUD(&hud_dashboard);
  }
}


void HUDShortcut::release() {
}


void HUDShortcut::pressKey(uint16_t key, uint8_t mode) {
  // ignore initial inputs
  if (!timeout(100)) {
    return;
  }

  if (key == 'f' || key == 'F') {
    Protogen.setEmotionShortcut(0, 0, (key == 'F') ? 1 : 0);
  }
  else if (key == 'd' || key == 'D') {
    Protogen.setEmotionShortcut(0, 1, (key == 'D') ? 1 : 0);
  }
  else if (key == 's' || key == 'S') {
    Protogen.setEmotionShortcut(0, 2, (key == 'S') ? 1 : 0);
  }
  else if (key == 'a' || key == 'A') {
  }
  else if (key == 'j' || key == 'J') {
    Protogen.setEmotionShortcut(1, 0, (key == 'J') ? 1 : 0);
  }
  else if (key == 'k' || key == 'K') {
    Protogen.setEmotionShortcut(1, 1, (key == 'K') ? 1 : 0);
  }
  else if (key == 'l' || key == 'L') {
    Protogen.setEmotionShortcut(1, 2, (key == 'L') ? 1 : 0);
  }
  else if (key == ';' || key == ':') {
  }
  else {
    return;
  }

  nextHUD(&hud_dashboard);
}


};
