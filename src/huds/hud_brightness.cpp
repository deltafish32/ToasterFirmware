#include "hud_brightness.h"


#include "huds.h"
#include "hud.h"
#include "protogen.h"


namespace toaster {

HUDBrightness hud_brightness;


static const unsigned char PROGMEM bmp_prev[] =
{ B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000100,
  B00000000, B00011100,
  B00000000, B01111100,
  B00000001, B11111100,
  B00000111, B11111100,
  B00011111, B11111100,
  B01111111, B11111100,
  B00011111, B11111100,
  B00000111, B11111100,
  B00000001, B11111100,
  B00000000, B01111100,
  B00000000, B00011100,
  B00000000, B00000100,
  B00000000, B00000000, };
static const int bmp_prev_w = 16;
static const int bmp_prev_h = 16;

static const unsigned char PROGMEM bmp_up[] =
{ B00000000, B00000000,
  B00000001, B00000000,
  B00000001, B00000000,
  B00000011, B10000000,
  B00000011, B10000000,
  B00000111, B11000000,
  B00000111, B11000000,
  B00001111, B11100000,
  B00001111, B11100000,
  B00011111, B11110000,
  B00011111, B11110000,
  B00111111, B11111000,
  B00111111, B11111000,
  B01111111, B11111100,
  B00000000, B00000000,
  B00000000, B00000000, };
static const int bmp_up_w = 16;
static const int bmp_up_h = 16;

static const unsigned char PROGMEM bmp_down[] =
{ B00000000, B00000000,
  B00000000, B00000000, 
  B01111111, B11111100,
  B00111111, B11111000,
  B00111111, B11111000,
  B00011111, B11110000,
  B00011111, B11110000,
  B00001111, B11100000,
  B00001111, B11100000,
  B00000111, B11000000,
  B00000111, B11000000,
  B00000011, B10000000,
  B00000011, B10000000,
  B00000001, B00000000,
  B00000001, B00000000,
  B00000000, B00000000,  };
static const int bmp_down_w = 16;
static const int bmp_down_h = 16;


void HUDBrightness::init() {
  HUDBase::init();
}


void HUDBrightness::process(Adafruit_SSD1306& oled) {
  switch (_step) {
  case 0:
    if (_dirty) {
      _dirty = false;
      
      oled.clearDisplay();

      oled.drawBitmap(0, (OLED_HEIGHT - bmp_prev_h) / 2, bmp_prev, bmp_prev_w, bmp_prev_h, 1);
      oled.drawBitmap((OLED_WIDTH - bmp_up_w) / 2, 0, bmp_up, bmp_up_w, bmp_up_h, 1);
      oled.drawBitmap((OLED_WIDTH - bmp_down_w) / 2, OLED_HEIGHT - bmp_down_h, bmp_down, bmp_down_w, bmp_down_h, 1);

      oled.setTextColor(WHITE);
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


void HUDBrightness::pressKey(uint16_t key) {
  if (key == 'H') {
    prevHUD();
  }
  if (key == 'J') {
    int brightness = getBrightness();
    brightness -= 25;
    if (brightness < 0) brightness = 0;
    uint8_t value = (uint8_t)(brightness / 100.0f * 255);
    Protogen._display.setBrightness(value);
    Protogen._side_display.setBrightness(value);
    setDirty();
  }
  if (key == 'K') {
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
