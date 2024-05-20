#include "config/configure.h"
#include "display.h"
#include "lib/logger.h"


namespace toaster {

static const char* TAG = "Display";


Display::Display() {
}


bool Display::begin(int width, int height) {
  _width = width;
  _height = height;

  if (_buffer != nullptr) {
    delete[] _buffer;
  }
  _buffer = new uint8_t[_width * _height * 3];

  return (_buffer != nullptr);
}


void Display::beginDraw() {
}


void Display::endDraw() {
  // _total_r = _total_g = _total_b = 0;

  // for (int i = 0; i < _width * _height * 3; i += 3) {
  //   _total_r += _buffer[i + 0];
  //   _total_g += _buffer[i + 1];
  //   _total_b += _buffer[i + 2];
  // }
}


void Display::clear() {
  memset(_buffer, 0, _width * _height * 3 * sizeof(uint8_t));
}


void Display::fill(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < _width * _height * 3; i += 3) {
    _buffer[i + 0] = r;
    _buffer[i + 1] = g;
    _buffer[i + 2] = b;
  }
}


void Display::fillWhite() {
  memset(_buffer, 255, _width * _height * 3 * sizeof(uint8_t));
}


bool Display::draw_image_newcolor(const Image* image, COLOR_FUNC color_func, uint8_t param, DRAW_MODE draw_mode, int offset_x, int offset_y, int rotate_cw) {
  if (image == nullptr) {
    TF_LOGE(TAG, "image draw failed (nullptr).");
    return false;
  }

  auto width = image->getWidth();
  auto height = image->getHeight();
  auto bpp = image->getBpp();
  auto has_alpha = image->getHasAlpha();
  auto buffer = image->getBuffer();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = (y * width + x) * (bpp + has_alpha);
      uint8_t a = has_alpha ? buffer[index + bpp] : 255;
      if (a == 0) continue;
      
      uint8_t r1;
      uint8_t g1;
      uint8_t b1;

      if (bpp == 3) {
        r1 = buffer[index + 0];
        g1 = buffer[index + 1];
        b1 = buffer[index + 2];
      }
      else {
        Image::rgb565be_to_rgb888(*((uint16_t*)(buffer + index)), r1, g1, b1);
      }
      
      int xr, yr;
      xy_rotate(xr, yr, x, y, width, height, rotate_cw);
      
      int x1 = xr + offset_x;
      int y1 = yr + offset_y;

      color_func(x1, y1, r1, g1, b1, a, param);

      int index0 = (y1 * _width + x1) * 3;
      uint8_t r0 = _buffer[index0 + 0];
      uint8_t g0 = _buffer[index0 + 1];
      uint8_t b0 = _buffer[index0 + 2];

      uint8_t r2 = (uint8_t)(((uint16_t)r1 * a / 255) + ((uint16_t)r0 * (255 - a) / 255));
      uint8_t g2 = (uint8_t)(((uint16_t)g1 * a / 255) + ((uint16_t)g0 * (255 - a) / 255));
      uint8_t b2 = (uint8_t)(((uint16_t)b1 * a / 255) + ((uint16_t)b0 * (255 - a) / 255));
      setPixel(x1, y1, r2, g2, b2);
    }
  }

  return true;
}


};
