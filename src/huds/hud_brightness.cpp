#include "hud_brightness.h"


#include "huds.h"
#include "hud.h"
#include "protogen.h"


namespace toaster {

HUDBrightness hud_brightness;


void HUDBrightness::init() {
  HUDBase::init();
}


void HUDBrightness::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    if (1) {
      oled.clearDisplay();

      int timeout_width = OLED_WIDTH * (TIMEOUT_INACTIVE_MS - getTimeElapsed()) / TIMEOUT_INACTIVE_MS;
      timeout_width = std::min(std::max(timeout_width, 0), OLED_WIDTH);
      oled.drawFastHLine(0, 0, timeout_width, SSD1306_WHITE);

      oled.drawBitmap(0, (OLED_HEIGHT - BITMAP_PREV_HEIGHT) / 2, BITMAP_PREV, BITMAP_PREV_WIDTH, BITMAP_PREV_HEIGHT, 1);
      oled.drawBitmap(OLED_WIDTH - BITMAP_OK_WIDTH, (OLED_HEIGHT - BITMAP_OK_HEIGHT) / 2, BITMAP_OK, BITMAP_OK_WIDTH, BITMAP_OK_HEIGHT, SSD1306_WHITE);

      oled.drawBitmap((OLED_WIDTH - BITMAP_UP_WIDTH) / 2, 0, BITMAP_UP, BITMAP_UP_WIDTH, BITMAP_UP_HEIGHT, 1);
      oled.drawBitmap((OLED_WIDTH - BITMAP_DOWN_WIDTH) / 2, OLED_HEIGHT - BITMAP_DOWN_WIDTH, BITMAP_DOWN, BITMAP_DOWN_WIDTH, BITMAP_DOWN_HEIGHT, 1);

      setFont(oled);
      oled.setCursor(12 * 4, 24);

      char str[16];
      snprintf(str, sizeof(str), "%3d", getBrightness());
      oled.write(str);

      oled.display();
    }
    if (timeout(TIMEOUT_INACTIVE_MS)) {
      nextHUD(&hud_dashboard, true);
      // prevHUD();
    }
    break;
  }
}


void HUDBrightness::release() {
}


void HUDBrightness::pressKey(uint16_t key, uint8_t mode) {
  if (key == 's'
   || mode == KM_KEYCODE && key == VK_DOWN) {
    int brightness = getBrightness();
    brightness -= 25;
    if (brightness < 0) brightness = 0;
    Protogen.setManualBrightness(brightness / 100.0f);
    
    restartTimer();
  }
  else if (key == 'f'
   || mode == KM_KEYCODE && key == VK_UP) {
    int brightness = getBrightness();
    brightness += 25;
    if (brightness > 100) brightness = 100;
    Protogen.setManualBrightness(brightness / 100.0f);
    
    restartTimer();
  }
  else {
    prevHUD();
  }
}


int HUDBrightness::getBrightness() {
  return (int)(Protogen._display.getBrightness() / 255.0f * 1000 + 5) / 10;
}

};
