#pragma once
#include <stdint.h>
#include <stddef.h>
#include <FFat.h>
#include "timer.h"


namespace toaster {


class Image {
public:
  Image(const char* path, bool rgb565);
  Image(uint8_t type, const uint8_t* buffer, size_t size, bool rgb565);
  Image(size_t width, size_t height, uint8_t bpp, uint8_t has_alpha);
  virtual ~Image();

public:
  void release();
  bool isLoaded();

  uint8_t* getBuffer() {
    return _buffer;
  }

  const uint8_t* getBuffer() const {
    return _buffer;
  }

  size_t getWidth() const {
    return _width;
  }

  size_t getHeight() const {
    return _height;
  }

  uint8_t getBpp() const {
    return _bpp;
  }

  uint8_t getHasAlpha() const {
    return _has_alpha;
  }

  void clear() {
    if (_buffer != nullptr) {
      memset(_buffer, 0, _size);
    }
  }

public:
  enum IMAGE_TYPE {
    IMAGE_NONE = 0,
    IMAGE_PNG,
    IMAGE_JPEG,
  };

public:
  static inline void rgb565be_to_rgb888(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = (((((uint32_t)rgb565 >> 11) & 0x1f) * 527) + 23) >> 6;
    g = (((((uint32_t)rgb565 >>  5) & 0x3f) * 259) + 33) >> 6;
    b = (((((uint32_t)rgb565 >>  0) & 0x1f) * 527) + 23) >> 6;
  }
  
  static inline uint16_t rgb888_to_rgb565be(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
  }

protected:
  size_t _width{0};
  size_t _height{0};

  uint8_t _type{IMAGE_NONE};
  uint8_t _bpp{0}; // 2: RGB565, 3: RGB888
  uint8_t _has_alpha{0};
  uint8_t _spare0{0};
  
  uint8_t* _buffer{nullptr};
  size_t _size{0};

protected:
  bool _load_png(void*, bool rgb565);
  bool load_png_from_bytes(const uint8_t* buffer, size_t size, bool rgb565);
  bool load_png(const char* path, bool rgb565);
  bool load_jpeg_from_bytes(const uint8_t* buffer, size_t size);
  bool load_jpeg(const char* path);

};


};
