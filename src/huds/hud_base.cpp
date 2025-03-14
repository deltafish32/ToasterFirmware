#include "hud_base.h"

namespace toaster {

const timer_ms_t HUDBase::TIMEOUT_INACTIVE_MS = 3000;


const size_t BITMAP_PREV_WIDTH = 16;
const size_t BITMAP_PREV_HEIGHT = 16;
const uint8_t PROGMEM BITMAP_PREV[] =
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


const size_t BITMAP_UP_WIDTH = 16;
const size_t BITMAP_UP_HEIGHT = 16;
const uint8_t PROGMEM BITMAP_UP[] =
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


const size_t BITMAP_DOWN_WIDTH = 16;
const size_t BITMAP_DOWN_HEIGHT = 16;
const uint8_t PROGMEM BITMAP_DOWN[] =
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


const uint8_t PROGMEM BITMAP_CELSIUS[] = {
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


const uint8_t PROGMEM BITMAP_EX_MARK[] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x06, 0x00, 
  0x06, 0x00, 
  0x00, 0x00,
};


const uint8_t PROGMEM BITMAP_LOCK[] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x0f, 0x00, 
  0x10, 0x80, 
  0x10, 0x80, 
  0x10, 0x80, 
  0x10, 0x80, 
  0x7f, 0xe0, 
  0x7f, 0xe0, 
  0x79, 0xe0, 
  0x79, 0xe0, 
  0x7d, 0xe0, 
  0x7f, 0xe0, 
  0x7f, 0xe0, 
  0x00, 0x00, 
  0x00, 0x00,
};


const uint8_t PROGMEM BITMAP_PLAY[3][32] = {
  {
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x38, 0x00, 
    0x3b, 0x60, 
    0x3b, 0x60, 
    0x38, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00,
  },
  {
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x07, 0x00, 
    0x37, 0x60, 
    0x37, 0x60, 
    0x07, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00,
  },
  {
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0xe0, 
    0x36, 0xe0, 
    0x36, 0xe0, 
    0x00, 0xe0, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00,
  },
};


const uint8_t PROGMEM BITMAP_PAUSE[] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x19, 0x80, 
  0x00, 0x00,
};


const uint8_t PROGMEM BITMAP_SHUFFLE[] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x40, 
  0x00, 0x60, 
  0x61, 0xf0, 
  0x12, 0x60, 
  0x08, 0x40, 
  0x08, 0x00, 
  0x04, 0x00, 
  0x04, 0x40, 
  0x12, 0x60, 
  0x61, 0xf0, 
  0x00, 0x60, 
  0x00, 0x40, 
  0x00, 0x00, 
  0x00, 0x00,
};


const uint8_t PROGMEM BITMAP_BACK[] = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x08, 0x00, 
  0x18, 0x00, 
  0x38, 0x00, 
  0x7f, 0x80, 
  0x38, 0x40, 
  0x18, 0x20, 
  0x08, 0x20, 
  0x00, 0x20, 
  0x00, 0x20, 
  0x00, 0x40, 
  0x01, 0x80, 
  0x0e, 0x00, 
  0x00, 0x00, 
  0x00, 0x00,
};


const uint8_t PROGMEM BITMAP_CURSOR[] = {
  0x00, 0x00, 
  0x40, 0x00, 
  0x60, 0x00, 
  0x70, 0x00, 
  0x78, 0x00, 
  0x7c, 0x00, 
  0x7e, 0x00, 
  0x7f, 0x00, 
  0x7f, 0x00, 
  0x7e, 0x00, 
  0x7c, 0x00, 
  0x78, 0x00, 
  0x70, 0x00, 
  0x60, 0x00, 
  0x40, 0x00, 
  0x00, 0x00,
};


};

#include "huds.h"
