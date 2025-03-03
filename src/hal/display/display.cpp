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


void Display::fillHalf(uint8_t lr, uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < _height; i++) {
    for (int j = 0; j < (_width / 2); j++) {
      int index = (i * _width) + ((j + (lr * _width / 2)) * 3);
      _buffer[index + 0] = r;
      _buffer[index + 1] = g;
      _buffer[index + 2] = b;
    }
  }
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

      setPixelAlpha(x1, y1, r1, g1, b1, a);
    }
  }

  return true;
}


bool Display::draw_image_newcolor_ex(const Image* image, COLOR_FUNC color_func, uint8_t param, DRAW_MODE draw_mode, 
  int offset_x, int offset_y, int w, int h, int source_x, int source_y, int rotate_cw) {

  if (image == nullptr) {
    TF_LOGE(TAG, "image draw failed (nullptr).");
    return false;
  }

  auto image_width = image->getWidth();
  auto width = std::min(w, (int)image_width);
  auto height = std::min(h, (int)image->getHeight());
  auto bpp = image->getBpp();
  auto has_alpha = image->getHasAlpha();
  auto buffer = image->getBuffer();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = ((y + source_y) * image_width + (x + source_x)) * (bpp + has_alpha);
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

      setPixelAlpha(x1, y1, r1, g1, b1, a);
    }
  }

  return true;
}


};
