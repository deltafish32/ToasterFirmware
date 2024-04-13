#pragma once

#include <vector>
#include "lib/upng.h"


namespace toaster {

typedef enum _DRAW_MODE {
  DRAW_DEFAULT = 0,

  DRAW_SINGLE = 0,
  DRAW_COPY,
  DRAW_MIRROR,
  DRAW_MIRROR_ONLY_OFFSET,

  DRAW_MAX
} DRAW_MODE;


typedef std::function<void(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param)> COLOR_FUNC;



class Display {
public:
  Display();

public:
  virtual bool begin(int width, int height);

public:
  virtual void beginDraw();
  virtual void endDraw();

  void clear();
  void fill(uint8_t r, uint8_t g, uint8_t b);
  void fillWhite();

public:
  inline void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    int index = (y * _width + x) * 3;
    _buffer[index + 0] = r;
    _buffer[index + 1] = g;
    _buffer[index + 2] = b;
  }

  inline uint32_t getPixel(int x, int y) const {
    int index = (y * _width + x) * 3;
    return (_buffer[index + 0] | (_buffer[index + 1] << 8) | (_buffer[index + 2] << 16));
  }

  inline uint8_t getPixelR(int x, int y) const {
    int index = (y * _width + x) * 3;
    return (_buffer[index + 0]);
  }

  inline uint8_t getPixelG(int x, int y) const {
    int index = (y * _width + x) * 3;
    return (_buffer[index + 1]);
  }

  inline uint8_t getPixelB(int x, int y) const {
    int index = (y * _width + x) * 3;
    return (_buffer[index + 2]);
  }

  inline bool getPixelOn(int x, int y) const {
    int index = (y * _width + x) * 3;
    return _buffer[index + 0] || _buffer[index + 1] || _buffer[index + 2];
  }
  
  inline bool getPixelOnThreshold(int x, int y) const {
    int index = (y * _width + x) * 3;
    return (_buffer[index + 0] >= 128) || (_buffer[index + 1] >= 128) || (_buffer[index + 2] >= 128);
  }

  // uint32_t getTotalR() const {
  //   return _total_r;
  // }

  // uint32_t getTotalG() const {
  //   return _total_g;
  // }

  // uint32_t getTotalB() const {
  //   return _total_b;
  // }

public:
  static upng_t* load_png(const char* name);
  static void unload_png(upng_t* upng);

  virtual bool draw_png(upng_t* upng, DRAW_MODE draw_mode = DRAW_DEFAULT, int offset_x = 0, int offset_y = 0, int rotate_cw = 0, uint8_t param = 0) {
    return draw_png_newcolor(upng, [](int, int, uint8_t&, uint8_t&, uint8_t&, uint8_t, uint8_t) {}, param, draw_mode, offset_x, offset_y, rotate_cw);
  }
  virtual bool draw_png_newcolor(upng_t* upng, COLOR_FUNC color_func, uint8_t param = 0, DRAW_MODE draw_mode = DRAW_DEFAULT, int offset_x = 0, int offset_y = 0, int rotate_cw = 0);

  // virtual bool undraw_png(upng_t* upng, DRAW_MODE draw_mode = DRAW_DEFAULT, int offset_x = 0, int offset_y = 0, int rotate_cw = 0);

protected:
  uint8_t* _buffer{nullptr};
  size_t _width{0};
  size_t _height{0};

// protected:
//   uint32_t _total_r{0};
//   uint32_t _total_g{0};
//   uint32_t _total_b{0};

protected:
  uint8_t _brightness{64};

public:
  uint8_t getBrightness() const {
    return _brightness;
  }
  void setBrightness(uint8_t brightness) {
    _brightness = brightness;
  }

  size_t getWidth() const {
    return _width;
  }

  size_t getHeight() const {
    return _height;
  }

protected:
  static inline void xy_rotate(int& out_x, int& out_y, int x, int y, int w, int h, int rotate_cw) {
    if (rotate_cw == 0) { // 0
      out_x = x;
      out_y = y;
    }
    else if (rotate_cw == 1) { // cw 90
      out_x = h - y - 1;
      out_y = x;
    }
    else if (rotate_cw == 2) { // 180
      out_x = w - x - 1;
      out_y = h - y - 1;
    }
    else if (rotate_cw == 3) { // cw 270
      out_x = y;
      out_y = w - x - 1;
    }
  }

};

};
