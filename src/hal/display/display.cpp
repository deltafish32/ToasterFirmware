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


upng_t* Display::load_png(const char* name) {
  timer_us_t tick = Timer::get_micros();
  upng_t* upng = upng_new_from_file(name);

  if (upng == nullptr) {
    TF_LOGE(TAG, "upng load failed (%s).", name);
    return nullptr;
  }
  
  timer_us_t tick2 = Timer::get_micros();

  upng_decode(upng);

  auto error = upng_get_error(upng);
  if (error != UPNG_EOK) {
    TF_LOGE(TAG, "upng decode failed (%s) (%d).", name, error);
    return nullptr;
  }

  timer_us_t tick3 = Timer::get_micros();
  //TF_LOGD(TAG, "upng load (%s, %lld us, %lld us)", name, tick2 - tick, tick3 - tick2);

  return upng;
}


void Display::unload_png(upng_t* upng) {
  if (upng == nullptr) {
    //TF_LOGE(TAG, "upng unload failed.");
    return;
  }

  // TF_LOGD(TAG, "upng unload");

  upng_free(upng);
}


bool Display::draw_png_newcolor(upng_t* upng, COLOR_FUNC color_func, uint8_t param, DRAW_MODE draw_mode, int offset_x, int offset_y, int rotate_cw) {
  if (upng == nullptr) {
    TF_LOGE(TAG, "upng draw failed (nullptr).");
    return false;
  }

  auto format = upng_get_format(upng);

  if (format != UPNG_RGB8 && format != UPNG_RGBA8) {
    TF_LOGE(TAG, "upng unsupported format (%d).", format);
    return false;
  }

  auto width = upng_get_width(upng);
  auto height = upng_get_height(upng);
  // auto size = upng_get_size(upng);
  // auto bitdepth = upng_get_bitdepth(upng);
  auto components = upng_get_components(upng);
  auto buffer = upng_get_buffer(upng);
  
  //TF_LOGD(TAG, "%d x %d, size: %d, bitdepth: %d, components: %d\n", width, height, size, bitdepth, components);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = (y * width + x) * components;
      uint8_t a = (components >= 4) ? buffer[index + 3] : 255;
      if (a == 0) continue;
      
      uint8_t r1 = buffer[index + 0];
      uint8_t g1 = buffer[index + 1];
      uint8_t b1 = buffer[index + 2];
      
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


// bool Display::undraw_png(upng_t* upng, DRAW_MODE draw_mode, int offset_x, int offset_y, int rotate_cw) {
//   if (upng == nullptr) {
//     TF_LOGE(TAG, "upng draw failed (nullptr).");
//     return false;
//   }

//   auto format = upng_get_format(upng);

//   if (format != UPNG_RGB8 && format != UPNG_RGBA8) {
//     TF_LOGE(TAG, "upng unsupported format (%d).", format);
//     return false;
//   }

//   auto width = upng_get_width(upng);
//   auto height = upng_get_height(upng);
//   // auto size = upng_get_size(upng);
//   // auto bitdepth = upng_get_bitdepth(upng);
//   auto components = upng_get_components(upng);
//   auto buffer = upng_get_buffer(upng);
  
//   //TF_LOGD(TAG, "%d x %d, size: %d, bitdepth: %d, components: %d", width, height, size, bitdepth, components);

//   for (int y = 0; y < height; y++) {
//     for (int x = 0; x < width; x++) {
//       int index = (y * width + x) * components;

//       uint8_t a = (components >= 4) ? buffer[index + 3] : 255;

//       if (a == 0) continue;
      
//       int xr, yr;
//       xy_rotate(xr, yr, x, y, width, height, rotate_cw);
      
//       int x1 = xr + offset_x;
//       int y1 = yr + offset_y;

//       uint8_t r2 = (uint8_t)((uint16_t)getPixelR(x1, y1) * (255 - a) / 255);
//       uint8_t g2 = (uint8_t)((uint16_t)getPixelG(x1, y1) * (255 - a) / 255);
//       uint8_t b2 = (uint8_t)((uint16_t)getPixelB(x1, y1) * (255 - a) / 255);
//       setPixel(x1, y1, r2, g2, b2);
//     }
//   }

//   return true;
// }

};
