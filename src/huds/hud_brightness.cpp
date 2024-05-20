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
    if (_dirty) {
      _dirty = false;
      
      oled.clearDisplay();

      oled.drawBitmap(0, (OLED_HEIGHT - BITMAP_PREV_HEIGHT) / 2, BITMAP_PREV, BITMAP_PREV_WIDTH, BITMAP_PREV_HEIGHT, 1);
      oled.drawBitmap((OLED_WIDTH - BITMAP_UP_WIDTH) / 2, 0, BITMAP_UP, BITMAP_UP_WIDTH, BITMAP_UP_HEIGHT, 1);
      oled.drawBitmap((OLED_WIDTH - BITMAP_DOWN_WIDTH) / 2, OLED_HEIGHT - BITMAP_DOWN_WIDTH, BITMAP_DOWN, BITMAP_DOWN_WIDTH, BITMAP_DOWN_HEIGHT, 1);

      oled.setTextColor(SSD1306_WHITE);
      oled.setTextSize(2);
      oled.setCursor(12 * 4, 24);

      char str[16];
      sprintf(str, "%3d", getBrightness());
      oled.write(str);

      oled.display();
    }
    break;
  }
}


void HUDBrightness::release() {
}


void HUDBrightness::pressKey(uint16_t key, uint8_t mode) {
  if (key == 'a'
   || mode == KM_KEYCODE && key == VK_LEFT) {
    prevHUD();
  }
  if (key == 's'
   || mode == KM_KEYCODE && key == VK_DOWN) {
    int brightness = getBrightness();
    brightness -= 25;
    if (brightness < 0) brightness = 0;
    uint8_t value = (uint8_t)(brightness / 100.0f * 255);
    Protogen._display.setBrightness(value);
    Protogen._side_display.setBrightness(value);
    setDirty();
  }
  if (key == 'f'
   || mode == KM_KEYCODE && key == VK_UP) {
    int brightness = getBrightness();
    brightness += 25;
    if (brightness > 100) brightness = 100;
    uint8_t value = (uint8_t)(brightness / 100.0f * 255);
    Protogen._display.setBrightness(value);
    Protogen._side_display.setBrightness(value);
    setDirty();
  }
}


int HUDBrightness::getBrightness() {
  return (int)(Protogen._display.getBrightness() / 255.0f * 1000 + 5) / 10;
}

};
