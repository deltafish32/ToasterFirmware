#include "hud_dashboard.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDDashboard hud_dashboard;

static const int ADC_MAX = 4095;

static const int UNLOCK_COUNT = 3;
static const uint32_t UNLOCK_TIMEOUT_MS = 3000;

static const uint8_t PROGMEM CELSIUS_BMP[] = {
  0x60, 0x00, 
  0x90, 0x00, 
  0x90, 0x00, 
  0x63, 0xc0, 
  0x06, 0x60, 
  0x0c, 0x60, 
  0x0c, 0x00, 
  0x0c, 0x00, 
  0x0c, 0x00, 
  0x0c, 0x00, 
  0x0c, 0x00, 
  0x0c, 0x60, 
  0x06, 0x60, 
  0x03, 0xc0, 
  0x00, 0x00, 
  0x00, 0x00, 
};


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

      oled.setCursor(12 * 0, 0);
      sprintf(sz, "%c", Protogen.getStaticMode() ? 'S' : ' ');
      oled.write(sz);
      
      oled.setCursor(12 * 1, 0);
      sprintf(sz, "%c", Protogen._boopsensor.isBoop() ? 'B' : ' ');
      oled.write(sz);

      if (Protogen._thermometer.isBegin()) {
        float temperature = Protogen._thermometer.getTemperature();
        float humidity = Protogen._thermometer.getHumidity();

        oled.setCursor(12 * 5 + 8, 0);
        sprintf(sz, "%.1f", temperature);
        oled.write(sz);
        
        oled.drawBitmap(12 * 9 + 8, 0, CELSIUS_BMP, 12, 16, 1);

        oled.setCursor(12 * 7 + 8, 16 * 1);
        sprintf(sz, "%.f%%", humidity);
        oled.write(sz);
      }
    }

    if (_unlock_count > 0) {
      char sz[32];
      oled.setCursor(12 * 2, 0);
      sprintf(sz, "%d", UNLOCK_COUNT - _unlock_count);
      oled.write(sz);
    }

    oled.display();
    
    if (Timer::get_millis() - _unlock_tick_ms >= UNLOCK_TIMEOUT_MS) {
      _unlock_count = 0;
    }
    
    break;
  }
}


void HUDDashboard::release() {
}


void HUDDashboard::pressKey(uint16_t key) {
  _unlock_tick_ms = millis();

  switch (key) {
  case 'A':
    Protogen.setStaticMode(!Protogen.getStaticMode());
    return;
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
  const size_t WIDTH = HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN;
  const size_t HEIGHT = HUB75_PANEL_RES_Y;
  const size_t BYTE_WIDTH = (WIDTH + 7) / 8;
  auto& display = Protogen._display;

  uint8_t buffer[BYTE_WIDTH * HEIGHT] = {0, };

  Protogen.syncLock();

  for (size_t i = 0; i < HEIGHT; i++) {
    for (size_t j = 0; j < WIDTH; j++) {
      size_t x_major = j / 8;
      size_t x_minor = j % 8;

      buffer[(i * BYTE_WIDTH) + x_major] |= (display.getPixelOnThreshold(j, i)) ? (0x80 >> x_minor) : 0;
    }
  }

  Protogen.syncUnlock();

  oled.drawBitmap(offset_x, offset_y, buffer, WIDTH, HEIGHT, WHITE);
}

};
