#include "config/configure.h"
#include "image.h"
#include "logger.h"
#include "upng.h"
#include <JPEGDEC.h>
#include "ram.h"


#ifdef USE_SD
#include "SD.h"
#endif


namespace toaster {

static const char* TAG = "Image";


Image::Image(const char* path, bool from_sd, bool rgb565) {
  const char* ext = strrchr(path, '.');
  if (ext != nullptr) {
    if (strcasecmp(ext + 1, "png") == 0) {
      _type = IMAGE_PNG;
      load_png(path, from_sd, rgb565);
    }

    if (strcasecmp(ext + 1, "jpg") == 0 || strcasecmp(ext + 1, "jpeg") == 0) {
      _type = IMAGE_JPEG;
      load_jpeg(path, from_sd);
    }
  }
}


Image::Image(uint8_t type, const uint8_t* buffer, size_t size, bool rgb565) {
  _type = type;
  
  if (_type == IMAGE_PNG) {
    load_png_from_bytes(buffer, size, rgb565);
  }

  if (_type == IMAGE_JPEG) {
    load_jpeg_from_bytes(buffer, size);
  }
}


Image::Image(size_t width, size_t height, uint8_t bpp, uint8_t has_alpha) {
  _type = IMAGE_PNG;

  _width = width;
  _height = height;
  _bpp = bpp;
  _has_alpha = has_alpha;
  _size = _width * _height * (_bpp + _has_alpha);

  _buffer = (uint8_t *)malloc_auto(_size);
  if (_buffer == nullptr) {
    TF_LOGE(TAG, "image memory allocation failed (%d x %d).", (int)width, (int)height);
    release();
  }
}


Image::~Image() {
  release();
}


void Image::release() {
  if (_buffer != nullptr) {
    free_auto(_buffer);
    _buffer = nullptr;
  }

  _width = 0;
  _height = 0;
  _size = 0;
}


bool Image::isLoaded() const {
  return (_buffer != nullptr);
}


bool Image::_load_png(void* ptr, bool rgb565) {
  auto upng = (upng_t *)ptr;
  
  auto result = upng_header(upng);
  if (result != UPNG_EOK) {
    TF_LOGE(TAG, "png header failed (%d).", result);
    upng_free(upng);
    return false;
  }
  
  auto format = upng_get_format(upng);
  if (format != UPNG_RGB8 && format != UPNG_RGBA8) {
    TF_LOGE(TAG, "png unsupported format (%d).", format);
    upng_free(upng);
    return false;
  }

  _width = upng_get_width(upng);
  _height = upng_get_height(upng);
  _bpp = rgb565 ? 2 : 3;
  _has_alpha = (upng_get_components(upng) == 4) ? 1 : 0;
  _size = _width * _height * (_bpp + _has_alpha);

  _buffer = (uint8_t *)malloc_auto(_size);
  if (_buffer == nullptr) {
    TF_LOGE(TAG, "png memory allocation failed.");
    upng_free(upng);
    return false;
  }

  uint8_t* buffer_rgb888 = nullptr;

  if (rgb565) {
    size_t rgb888_size = _width * _height * (3 + _has_alpha);
    buffer_rgb888 = (uint8_t *)malloc_auto(rgb888_size);
    if (buffer_rgb888 == nullptr) {
      TF_LOGE(TAG, "png memory allocation failed.");
      release();
      upng_free(upng);
      return false;
    }
  }

  result = upng_decode(upng, rgb565 ? buffer_rgb888 : _buffer);
  if (result != UPNG_EOK) {
    TF_LOGE(TAG, "png decode failed (%d).", result);
    free_auto(buffer_rgb888);
    release();
    upng_free(upng);
    return false;
  }

  if (rgb565) {
    // for (int y = 0; y < _height; y++) {
    //   for (int x = 0; x < _width; x++) {
    //     int index_in = (y * _width + x) * (3 + _has_alpha);
    //     int index_out = (y * _width + x) * (2 + _has_alpha);
    //     uint16_t* p = (uint16_t*)(_buffer + index_out);
    //     *p = rgb888_to_rgb565be(buffer_rgb888[index_in + 0], buffer_rgb888[index_in + 1], buffer_rgb888[index_in + 2]);
    //     if (_has_alpha) {
    //       _buffer[index_out + 2] = buffer_rgb888[index_in + 3];
    //     }
    //   }
    // }
    
    uint8_t *ptr_in = buffer_rgb888;
    uint8_t *ptr_out = _buffer;
    const int pixels = _width * _height;
    
    if (_has_alpha) {
      for (int i = 0; i < pixels; i++) {
        *((uint16_t*)ptr_out) = rgb888_to_rgb565be(ptr_in[0], ptr_in[1], ptr_in[2]);
        ptr_out[2] = ptr_in[3];
        ptr_in += 4;
        ptr_out += 3;
      }
    }
    else {
      for (int i = 0; i < pixels; i++) {
        *((uint16_t*)ptr_out) = rgb888_to_rgb565be(ptr_in[0], ptr_in[1], ptr_in[2]);
        ptr_in += 3;
        ptr_out += 2;
      }
    }
    
    free_auto(buffer_rgb888);
  }
  
  upng_free(upng);

  return true;
}


bool Image::load_png_from_bytes(const uint8_t* buffer, size_t size, bool rgb565) {
  release();

  upng_t* upng = upng_new_from_bytes(buffer, size);
  if (upng == nullptr) {
    TF_LOGE(TAG, "upng load failed.");
    return false;
  }

  return _load_png(upng, rgb565);
}


bool Image::load_png(const char* path, bool from_sd, bool rgb565) {
  release();

  upng_t* upng = nullptr;
  if (from_sd) {
#ifdef USE_SD
    upng = upng_new_from_file(path, SD);
#endif
  }
  else {
    upng = upng_new_from_file(path, FFat);
  }
  if (upng == nullptr) {
    TF_LOGE(TAG, "upng load failed (%s).", path);
    return false;
  }

  return _load_png(upng, rgb565);
}


bool Image::load_jpeg_from_bytes(const uint8_t* buffer, size_t size) {
  release();

  auto jpegdec = (JPEGDEC *)malloc_auto(sizeof(JPEGDEC));
  if (jpegdec == nullptr) {
    TF_LOGE(TAG, "JPEGDEC initialization failed.");
    return false;
  }
  
  if (!jpegdec->openRAM((uint8_t*)buffer, size, [](JPEGDRAW *draw) -> int {
    auto image = ((Image*)draw->pUser);

    if (draw->iBpp != 16) {
      return 0;
    }

    uint16_t* ptr_in = draw->pPixels;
    uint16_t* ptr_out = (uint16_t*)image->_buffer;

    for (int y = 0; y < draw->iHeight; y++) {
      int index_base = (y + draw->y) * image->_width + draw->x;
      for (int x = 0; x < draw->iWidthUsed; x++) {
        ptr_out[index_base + x] = *ptr_in++;
      }
      
      ptr_in += (draw->iWidth - draw->iWidthUsed);
    }

    return 1;
  })) {
    TF_LOGE(TAG, "JPEGDEC openRAM failed (%d).", jpegdec->getLastError());
    jpegdec->close();
    free_auto(jpegdec);
    return false;
  }

  _width = jpegdec->getWidth();
  _height = jpegdec->getHeight();
  _bpp = 2;
  _has_alpha = 0;
  _size = _width * _height * (_bpp + _has_alpha);
  
  _buffer = (uint8_t *)malloc_auto(_size);
  if (_buffer == nullptr) {
    TF_LOGE(TAG, "jpeg memory allocation failed.");
    jpegdec->close();
    free_auto(jpegdec);
    return false;
  }

  // TF_LOGD(TAG, "subsample: 0x%02x", jpegdec->getSubSample());
  
  jpegdec->setUserPointer(this);
  jpegdec->setPixelType(RGB565_LITTLE_ENDIAN);
  if (!jpegdec->decode(0, 0, 0)) {
    TF_LOGE(TAG, "JPEGDEC decode failed (%d).", jpegdec->getLastError());
    release();
    jpegdec->close();
    free_auto(jpegdec);
    return false;
  }

  jpegdec->close();
  free_auto(jpegdec);

  return true;
}


bool Image::load_jpeg(const char* path, bool from_sd) {
  File file;
  if (from_sd) {
#ifdef USE_SD
    file = SD.open(path);
#endif
  }
  else {
    file = FFat.open(path);
  }
  if (!file) {
    TF_LOGE(TAG, "jpeg load failed (%s).", path);
    return false;
  }

  size_t size = file.size();
  uint8_t* buffer = (uint8_t *)malloc_auto(size);
  if (buffer == nullptr) {
    TF_LOGE(TAG, "jpeg memory allocation failed (%s).", path);
    file.close();
    return false;
  }
  file.read(buffer, size);
  file.close();

  bool result = load_jpeg_from_bytes(buffer, size);
  free_auto(buffer);
  return result;
}


};
