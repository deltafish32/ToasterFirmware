#include "hud_dashboard.h"


#include "huds.h"
#include "protogen.h"

#include "lib/logger.h"
#include "lib/dither.h"


namespace toaster {

static const char* TAG = "HUDDashboard";

HUDDashboard hud_dashboard;

static const int ADC_MAX = 4095;

static const int UNLOCK_COUNT = 3;
static const uint32_t UNLOCK_TIMEOUT_MS = 3000;


void HUDDashboard::init() {
  HUDBase::init();

  _unlock_key = 0;
  _unlock_count = 0;
}


void HUDDashboard::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    oled.clearDisplay();
    oled.setCursor(0, 0);
    setFont(oled);

    if (_unlock_count > 0) {
      char sz[32];
      sprintf(sz, "%d", UNLOCK_COUNT - _unlock_count);
      oled.write(sz);
    }
    else if (Protogen.isBlank() == false) {
      writeSpecial(oled, BITMAP_LOCK);
    }

    if (Protogen.isBlank() == false) {
      mirror(oled, 0, 32, !Protogen.isGame() && Protogen._hud.getDithering());

      char sz[32];

      if (Protogen.getStaticMode()) {
        writeSpecial(oled, BITMAP_PAUSE);
      }
      else {
        int n = Timer::get_millis() / 200 % 4;
        if (n == 3) n = 1;
        writeSpecial(oled, BITMAP_PLAY[n]);
      }
      
      if (Protogen._boopsensor.isBoop()) {
        writeSpecial(oled, BITMAP_EX_MARK);
      }

      if (Protogen._thermometer.isBegin()) {
        float temperature = Protogen._thermometer.getTemperature();
        float humidity = Protogen._thermometer.getHumidity();

        oled.setCursor(12 * 5 + 8, 0);
        sprintf(sz, "%.1f", temperature);
        oled.write(sz);
        
        writeSpecial(oled, BITMAP_CELSIUS);

        oled.setCursor(12 * 7 + 8, 16 * 1);
        sprintf(sz, "%.f%%", humidity);
        oled.write(sz);
      }

      if (Protogen._rtc.isBegin()) {
        oled.setCursor(12 * 0, 16 * 1);
        oled.write(Protogen.getTimeStr());
      }
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


void HUDDashboard::pressKey(uint16_t key, uint8_t mode) {
  if (mode == 0) {
    if (key == 'f' || key == 'd' || key == 's' || key == 'j' || key == 'k' || key == 'l') {
      _unlock_tick_ms = millis();

      if (_unlock_key != key) {
        _unlock_key = key;
        _unlock_count = 1;
      }
      else {
        ++_unlock_count;
        if (_unlock_count >= UNLOCK_COUNT) {
          _unlock_count = 0;

          switch (_unlock_key) {
          case 'f': case 'j': nextHUD(&hud_main); break;
          case 'd': case 'k': Protogen.setStaticMode(!Protogen.getStaticMode()); break;
          case 's': case 'l': nextHUD(&hud_shortcut); break;
          }

        }
      }
    }
    else if (key == 'E' || key == 'e') {
      Protogen.setStaticMode(!Protogen.getStaticMode());
      TF_LOGI(TAG, "%s", Protogen.getStaticMode() ? "pause" : "play");
    }
  }
  else {
    switch (key) {
    case VK_RIGHT:
      nextHUD(&hud_main);
      break;
    }
  }
}


void HUDDashboard::mirror(Adafruit_SSD1306& oled, int offset_x, int offset_y, bool dither) {
  const size_t WIDTH = HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN;
  const size_t HEIGHT = HUB75_PANEL_RES_Y;
  const size_t BYTE_WIDTH = (WIDTH + 7) / 8;
  auto& display = Protogen._display;

  uint8_t* bitmap = new uint8_t[BYTE_WIDTH * HUB75_PANEL_RES_Y];
  uint8_t* gray = new uint8_t[HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN * HUB75_PANEL_RES_Y];

  memset(bitmap, 0, BYTE_WIDTH * HEIGHT);
  
  Protogen.syncLock();

  if (dither) {
    for (size_t i = 0; i < HEIGHT; i++) {
      for (size_t j = 0; j < WIDTH; j++) {
        gray[(i * WIDTH) + j] = display.getPixelGray(j, i);
      }
    }
  }
  else {
    for (size_t i = 0; i < HEIGHT; i++) {
      for (size_t j = 0; j < WIDTH; j++) {
        size_t x_major = j / 8;
        size_t x_minor = j % 8;

        bitmap[(i * BYTE_WIDTH) + x_major] |= (display.getPixelOnThreshold(j, i)) ? (0x80 >> x_minor) : 0;
      }
    }
  }

  Protogen.syncUnlock();

  if (dither) {
    fastEDDither(gray);

    for (size_t i = 0; i < HEIGHT; i++) {
      for (size_t j = 0; j < WIDTH; j++) {
        size_t x_major = j / 8;
        size_t x_minor = j % 8;

        bitmap[(i * BYTE_WIDTH) + x_major] |= ((gray[(i * WIDTH) + j]) & 0x80) ? (0x80 >> x_minor) : 0;
      }
    }
  }

  oled.drawBitmap(offset_x, offset_y, bitmap, WIDTH, HEIGHT, SSD1306_WHITE);

  delete[] bitmap;
  delete[] gray;
}

};
