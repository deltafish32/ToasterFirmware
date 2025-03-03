#include "config/configure.h"
#include "video.h"
#include "logger.h"
#include <AnimatedGIF.h>


namespace toaster {

static const char* TAG = "Video";


Video::Video(const char* path, uint32_t mjpeg_fps) {
  const char* ext = strrchr(path, '.');
  if (ext != nullptr) {
    if (strcasecmp(ext + 1, "gif") == 0) {
      _type = VIDEO_GIF;
      load_gif(path);
    }

    if (strcasecmp(ext + 1, "mjpeg") == 0) {
      _type = VIDEO_MJPEG;
      load_mjpeg(path, mjpeg_fps);
    }
  }
}


Video::~Video() {
  release();
}


void Video::release() {
  _file.close();

  if (_image != nullptr) {
    delete _image;
    _image = nullptr;
  }

  if (_gif != nullptr) {
    auto gif = (AnimatedGIF *)_gif;
    gif->close();
    delete gif;
    _gif = nullptr;
  }
}


bool Video::isLoaded() {
  return (_image != nullptr);
}


const Image* Video::getImage() const {
  return _image;
}


bool Video::nextFrame(bool loop, bool init) {
  if (_type == VIDEO_GIF) {
    return next_gif(loop, init);
  }
  
  if (_type == VIDEO_MJPEG) {
    return next_mjpeg(loop, init);
  }

  return false;
}


// Source referenced: https://github.com/pixelmatix/GifDecoder

static void* gif_file_open(const char *fname, int32_t *pSize) {
  File* file_ptr = new File;
  if (file_ptr == nullptr) {
    return nullptr;
  }

  *file_ptr = FFat.open(fname);
  if (!*file_ptr) {
    delete file_ptr;
    return nullptr;
  }

  *pSize = file_ptr->size();
  return (void *)file_ptr;
}


static void gif_file_close(void* param) {
  if (param == nullptr) {
    return;
  }

  File* file_ptr = (File *)param;
  file_ptr->close();
  delete file_ptr;
}


static int32_t gif_file_read(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  if (pFile == nullptr) {
    return 0;
  }
  
  File &file = *((File *)pFile->fHandle);
  int32_t bytes_read = file.readBytes((char*)pBuf, iLen);
  pFile->iPos = (int32_t)file.position();
  return bytes_read;
}


static int32_t gif_file_seek(GIFFILE *pFile, int32_t iPosition) {
  if (pFile == nullptr) {
    return 0;
  }
  
  File &file = *((File *)pFile->fHandle);
  file.seek(iPosition);
  pFile->iPos = (int32_t)file.position();
  return pFile->iPos;
}


static void gif_draw(GIFDRAW *pDraw) {
  auto image = (Image *)pDraw->pUser;
  if (image == nullptr) {
    return;
  }
  
  typedef struct _rgb888_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  } rgb888_t;
  
  auto palette = (const rgb888_t*)pDraw->pPalette;
  auto buffer = image->getBuffer();
  auto width = image->getWidth();
  const uint8_t ucTransparent = pDraw->ucTransparent;

  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency && pDraw->ucDisposalMethod != 2) { // if transparency used
    auto palette_temp = new rgb888_t[pDraw->iWidth];
    const uint8_t *ptr_color = pDraw->pPixels;
    const uint8_t *ptr_end = ptr_color + pDraw->iWidth;
    int x = 0;

    while (x < pDraw->iWidth) {
      uint8_t color = ucTransparent - 1;
      rgb888_t *ptr_palette = palette_temp;
      int opaque_count = 0; // count non-transparent pixels

      while (color != ucTransparent && ptr_color < ptr_end) {
        color = *ptr_color++;
        if (color == ucTransparent) { // done, stop
          ptr_color--; // back up to treat it like transparent
        }
        else { // opaque
          *ptr_palette++ = palette[color];
          opaque_count++;
        }
      } // while looking for opaque pixels

      if (opaque_count) { // any opaque pixels?
        auto ptr_buffer = buffer + (((pDraw->iY + pDraw->y) * width + (pDraw->iX + x)) * 3);
        for (int i = 0; i < opaque_count; i++) {
          *ptr_buffer++ = palette_temp[i].r;
          *ptr_buffer++ = palette_temp[i].g;
          *ptr_buffer++ = palette_temp[i].b;
        }

        x += opaque_count;
        opaque_count = 0;
      }

      // no, look for a run of transparent pixels
      color = ucTransparent;
      while (color == ucTransparent && ptr_color < ptr_end) {
        color = *ptr_color++;
        if (color == ucTransparent) {
          opaque_count++;
        }
        else {
          ptr_color--;
        }
      }

      if (opaque_count) {
        x += opaque_count; // skip these
        opaque_count = 0;
      }
    }

    delete[] palette_temp;
  }
  else {
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    auto ptr_buffer = buffer + (((pDraw->iY + pDraw->y) * width + (pDraw->iX)) * 3);
    for (int x = 0; x < pDraw->iWidth; x++) {
      uint8_t color = pDraw->pPixels[x];
      if (color == pDraw->ucTransparent) {
        *ptr_buffer++ = 0;
        *ptr_buffer++ = 0;
        *ptr_buffer++ = 0;
      }
      else {
        const auto& pal = palette[color];
        *ptr_buffer++ = pal.r;
        *ptr_buffer++ = pal.g;
        *ptr_buffer++ = pal.b;
      }
    }
  }
};


bool Video::load_gif(const char* path) {
  release();

  _file = FFat.open(path);
  if (!_file) {
    TF_LOGE(TAG, "gif file open failed.");
    return false;
  }

  auto gif = new AnimatedGIF;
  if (gif == nullptr) {
    TF_LOGE(TAG, "gif memory allocation failed.");
    release();
    return false;
  }

  gif->begin(GIF_PALETTE_RGB888);

  _path = path;
  
  if (!gif->open(_path.c_str(), gif_file_open, gif_file_close, gif_file_read, gif_file_seek, gif_draw)) {
    TF_LOGE(TAG, "gif decoding failed (%d).", gif->getLastError());
    delete gif;
    release();
    return false;
  }

  _type = VIDEO_GIF;
  auto width = gif->getCanvasWidth();
  auto height = gif->getCanvasHeight();
  _start = _end = 0;
  _gif = gif;

  if (_image != nullptr) {
    delete _image;
  }
  _image = new Image(width, height, 3, 0);
  if (_image == nullptr) {
    TF_LOGE(TAG, "gif memory allocation failed.");
    release();
    return false;
  }

  _image->clear();

  if (!load_gif_frame()) {
    release();
    return false;
  }

  return true;
}


bool Video::next_gif(bool loop, bool init) {
  if (_type != VIDEO_GIF) {
    return false;
  }

  auto gif = (AnimatedGIF *)_gif;
  if (!gif->nextFrame(init)) {
    if (loop) {
      _image->clear();
      return next_gif(loop, true);
    }
    
    return false;
  }

  return load_gif_frame();
}


bool Video::load_gif_frame() {
  if (_image == nullptr || _gif == nullptr) {
    TF_LOGE(TAG, "gif decode frame failed.");
    release();
    return false;
  }
  
  auto gif = (AnimatedGIF *)_gif;

  int frame_delay = 0;
  int gif_result = gif->playFrame(false, &frame_delay, _image);
  if (gif_result < 0) {
    TF_LOGE(TAG, "gif decode frame failed. (%d)", gif->getLastError());
    release();
    return false;
  }

  _pf.type = PF_PERIOD;
  _pf.period_ms = frame_delay;

  return true;
}


bool Video::load_mjpeg(const char* path, uint32_t fps) {
  release();

  _file = FFat.open(path);
  if (!_file) {
    TF_LOGE(TAG, "video load failed (%s).", path);
    return false;
  }

  _start = 0;

  if (!next_mjpeg(true)) {
    TF_LOGE(TAG, "video header not found (%s).", path);
    release();
    return false;
  }

  if (!load_mjpeg_frame()) {
    TF_LOGE(TAG, "video mjpeg first frame load failed. (%s)", path);
    release();
    return false;
  }

  _frame = 0;

  _pf.type = PF_FREQUENCY;
  _pf.frequency = fps;

  return true;
}


bool Video::next_mjpeg(bool loop, bool init) {
  if (init) {
    _start = 0;
    _frame = 0;
  }
  else {
    _start = _end;
    ++_frame;
  }

  _file.seek(_start);

  const size_t BLOCK_SIZE = CONFIG_WL_SECTOR_SIZE;
  uint8_t* block = new uint8_t[BLOCK_SIZE];

  int step = 0;
  bool find_header = false;
  bool find_next_header = false;

  while (_file.available() && find_next_header == false) {
    size_t block_pos = _file.position();
    size_t bytes_read = _file.readBytes((char*)block, BLOCK_SIZE);
    
    for (size_t i = 0; i < bytes_read && find_next_header == false; i++) {
      size_t pos = block_pos + i;
      uint8_t read = block[i];

      switch (step) {
      case 0: // find ff
        if (read == 0xff) {
          _start = pos;
          step = 1;
        }
        break;
      case 1: // find d8
        if (read == 0xd8) {
          find_header = true;
          step = 2;
        }
        else if (read == 0xff) {
          _start = pos;
        }
        else {
          step = 0;
        }
        break;
      case 2: // find next ff
        if (read == 0xff) {
          _end = pos;
          step = 3;
        }
        break;
      case 3: // find next d8
        if (read == 0xd8) {
          find_next_header = true;
          break;
        }
        else if (read == 0xff) {
          _end = pos;
        }
        else {
          step = 2;
        }
        break;
      }
    }
  }
  
  delete[] block;

  if (_image != nullptr) {
    delete _image;
    _image = nullptr;
  }

  bool has_next = find_header && (_start < _end);

  if (has_next) {
    if (!load_mjpeg_frame()) {
      return false;
    }
  }

  if (!has_next && loop) {
    return next_mjpeg(loop, true);
  }

  return has_next;
}


bool Video::load_mjpeg_frame() {
  if (_image != nullptr) {
    delete _image;
    _image = nullptr;
  }

  size_t size = _end - _start;
  uint8_t* buffer = new uint8_t[size];
  if (buffer == nullptr) {
    return false;
  }

  _file.seek(_start);
  _file.readBytes((char *)buffer, size);

  _image = new Image(Image::IMAGE_JPEG, buffer, size, true);
  if (_image == nullptr) {
    return false;
  }

  if (!_image->isLoaded()) {
    delete _image;
    return false;
  }
  
  delete[] buffer;

  return true;
}


};
