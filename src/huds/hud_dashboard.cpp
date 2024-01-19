#include "hud_dashboard.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDDashboard hud_dashboard;

static const int ADC_MAX = 4095;

static const int UNLOCK_COUNT = 3;
static const uint32_t UNLOCK_TIMEOUT = 3000;


void HUDDashboard::init() {
  HUDBase::init();

  _unlock_key = 0;
  _unlock_count = 0;
}


void HUDDashboard::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    oled.clearDisplay();

    if (Protogen.isBlank() == false) {
      mirror(oled, 0, 32);

      char sz[32];

      oled.setTextColor(WHITE);
      oled.setTextSize(2);
      
      // oled.setCursor(0, 0);
      // sprintf(sz, "%d", Protogen.getRecentFPS());
      // oled.write(sz);

      // oled.setCursor(12 * 5, 0);
      // sprintf(sz, "%d", Protogen._photoresistor.getValue());
      // oled.write(sz);

      // oled.setCursor(12 * 7, 0);
      // int brightness = round(Protogen._display.getBrightness() / 255.0f * 100.0f);
      // sprintf(sz, "%3d", brightness);
      // oled.write(sz);

      oled.setCursor(12 * 0, 0);
      int sensor = (int)round(Protogen._photoresistor.getValue() * 100.0f / ADC_MAX);
      if (sensor >= 100) {
        sprintf(sz, "SHH");
      }
      else {
        sprintf(sz, "S%2d", sensor);
      }
      oled.write(sz);

      oled.setCursor(12 * 4, 0);
      int brightness = (int)round(Protogen._display.getBrightness() * 100.0f / 255);
      if (brightness >= 100) {
        sprintf(sz, "BHH");
      }
      else {
        sprintf(sz, "B%2d", brightness);
      }
      oled.write(sz);

      oled.setCursor(12 * 8, 0);
      sprintf(sz, "%c", Protogen.getStaticMode() ? 'S' : ' ');
      oled.write(sz);
    }

    if (_unlock_count > 0) {
      char sz[32];
      oled.setCursor(12 * 9, 0);
      sprintf(sz, "%d", UNLOCK_COUNT - _unlock_count);
      oled.write(sz);
    }

    oled.display();
    
    if (millis() - _unlock_tick >= UNLOCK_TIMEOUT) {
      _unlock_count = 0;
    }
    
    break;
  }
}


void HUDDashboard::release() {
}


void HUDDashboard::pressKey(uint16_t key) {
  _unlock_tick = millis();

  switch (key) {
  case 'L':
    nextHUD(&hud_emotions);
    return;
  case 'S': break;
  case 'D': break;
  case 'F': break;
  default:
    return;
  }

  if (_unlock_key != key) {
    _unlock_key = key;
    _unlock_count = 1;
  }
  else {
    ++_unlock_count;
    if (_unlock_count >= UNLOCK_COUNT) {
      _unlock_count = 0;

      switch (_unlock_key) {
      case 'S': nextHUD(&hud_emotions); break;
      case 'D': Protogen.setStaticMode(!Protogen.getStaticMode()); break;
      case 'F': nextHUD(&hud_emotions); break;
      }

    }
  }
}


void HUDDashboard::mirror(Adafruit_SSD1306& oled, int offset_x, int offset_y) {
  Protogen.syncLock();

  auto& display = Protogen._display;
  size_t width = display.getWidth();
  size_t height = display.getHeight();

  for (size_t i = 0; i < height; i++) {
    for (size_t j = 0; j < width; j++) {
      if (display.getPixelOnThreshold(j, i)) {
        oled.drawPixel(j + offset_x, i + offset_y, WHITE);
      }
    }
  }

  Protogen.syncUnlock();
}

};
