#include "config/configure.h"
#include "video.h"
#include "logger.h"
#include <AnimatedGIF.h>
#include "ram.h"


#ifdef USE_SD
#include "SD.h"
#endif


namespace toaster {

static const char* TAG = "Video";


static const size_t MJPEG_BUFFER_SIZE = 64 * 1024;
static const size_t MJPEG_LOAD_FRAMES = 30;


Video::Video(const char* path, bool from_sd, bool loop, uint32_t mjpeg_fps) {
  const char* ext = strrchr(path, '.');
  if (ext != nullptr) {
    if (strcasecmp(ext + 1, "gif") == 0) {
      _type = VIDEO_GIF;
      load_gif(path, from_sd, loop);
    }

    if (strcasecmp(ext + 1, "mjpeg") == 0) {
      _type = VIDEO_MJPEG;
      load_mjpeg(path, from_sd, loop, mjpeg_fps);
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

  if (_type == VIDEO_MJPEG) {
    if (_buffering) {
      _buffering = false;
      while (!_mjpeg_read_end || !_mjpeg_decode_end) {
        delay(1);
      }
    }

    for (int i = 0; i < MJPEG_BUFFERS; i++) {
      if (_buffer[i].ptr != nullptr) {
        free_auto(_buffer[i].ptr);
        _buffer[i].ptr = nullptr;
      }
    }
  }

  for (auto it : _decoded) {
    delete it;
  }
  _decoded.clear();
  
  if (_interlock != nullptr) {
    vSemaphoreDelete(_interlock);
    _interlock = nullptr;
  }
}


bool Video::isLoaded() const {
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
static bool g_gif_from_sd = false;

static void* gif_file_open(const char *fname, int32_t *pSize) {
  File* file_ptr = new File;
  if (file_ptr == nullptr) {
    return nullptr;
  }

  if (g_gif_from_sd) {
#ifdef USE_SD
    *file_ptr = SD.open(fname);
#endif
  }
  else {
    *file_ptr = FFat.open(fname);
  }

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
  int32_t bytes_read = file.read(pBuf, iLen);
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
  if (buffer == nullptr) {
    return;
  }
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


bool Video::load_gif(const char* path, bool from_sd, bool loop) {
  release();

  g_gif_from_sd = from_sd;
  if (from_sd) {
#ifdef USE_SD
    _file = SD.open(path);
#endif
  }
  else {
    _file = FFat.open(path);
  }

  if (!_file) {
    TF_LOGE(TAG, "gif file open failed (sd: %d, %s).", from_sd, path);
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
  if (_image == nullptr || !_image->isLoaded()) {
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


bool Video::load_mjpeg(const char* path, bool from_sd, bool loop, uint32_t fps) {
  release();

  if (from_sd) {
#ifdef USE_SD
    _file = SD.open(path);
#endif
  }
  else {
    _file = FFat.open(path);
  }
  if (!_file) {
    TF_LOGE(TAG, "video load failed (sd: %d, %s).", from_sd ? 1 : 0, path);
    return false;
  }

  _start = 0;

  if (psramFound()) {
    _interlock = xSemaphoreCreateBinary();
    xSemaphoreGive(_interlock);

    for (int i = 0; i < MJPEG_BUFFERS; i++) {
      if (_buffer[i].ptr == nullptr) {
        _buffer[i].size = MJPEG_BUFFER_SIZE;
        _buffer[i].ptr = (uint8_t*)malloc_auto(_buffer[i].size);
        _buffer[i].start = 0;
        _buffer[i].read = 0;
        _buffer[i].valid = false;
        _buffer[i].last = false;
      }
    }

    _buffering = true;
    _buffering_loop = loop;
    _mjpeg_read_end = false;
    _mjpeg_decode_end = false;
    _buffering_fail = false;

    auto core_result1 = xTaskCreatePinnedToCore(mjpeg_read_entry_point, "mjpegr", 2048, this, 1, nullptr, PRO_CPU_NUM);
    if (core_result1 != pdPASS) {
      TF_LOGE(TAG, "xTaskCreatePinnedToCore failed (%d).", core_result1);
    }

    auto core_result2 = xTaskCreatePinnedToCore(mjpeg_decode_entry_point, "mjpegd", 8192, this, 1, nullptr, PRO_CPU_NUM);
    if (core_result2 != pdPASS) {
      TF_LOGE(TAG, "xTaskCreatePinnedToCore failed (%d).", core_result2);
    }

    if (!next_mjpeg(false, true)) {
      TF_LOGE(TAG, "video header not found (%s).", path);
      release();
      return false;
    }

    while (_decoded.size() < MJPEG_LOAD_FRAMES) {
      delay(1);
    }
  }
  else {
    if (!next_mjpeg(false, true)) {
      TF_LOGE(TAG, "video header not found (%s).", path);
      release();
      return false;
    }

    if (!load_mjpeg_frame()) {
      TF_LOGE(TAG, "video mjpeg first frame load failed. (%s)", path);
      release();
      return false;
    }
  }

  _frame = 0;

  _pf.type = PF_FREQUENCY;
  _pf.frequency = fps;

  return true;
}


bool Video::next_mjpeg(bool loop, bool init) {
  if (init) {
    _start = _end = 0;
    _frame = 0;
    
    if (psramFound()) {
      for (int i = 0; i < MJPEG_BUFFERS; i++) {
        if (_buffer[i].valid && (_buffer[i].last || _buffer[i].start > MJPEG_BUFFER_SIZE)) {
          _buffer[i].valid = false;
        }
      }
    }
  }
  else {
    _start = _end;
    ++_frame;
  }

  if (psramFound()) {
    timer_ms_t tick = Timer::get_millis();
    while (Timer::get_millis() - tick < 500) {
      if (_buffering_fail) {
        return false;
      }

      if (load_mjpeg_frame()) {
        return true;
      }

      delay(1);
    }
    
    return false;
  }
  else {
    bool has_next = find_range_direct() && (_start < _end);

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
}


bool Video::load_mjpeg_frame() {
  if (_image != nullptr) {
    delete _image;
    _image = nullptr;
  }
  
  if (psramFound()) {
    if (xSemaphoreTake(_interlock, portMAX_DELAY)) {
      if (!_decoded.empty()) {
        _image = _decoded.front();
        _decoded.pop_front();
      }
      xSemaphoreGive(_interlock);
    }

    return (_image != nullptr);
  }
  else {
    size_t size = _end - _start;
    uint8_t* buffer = new uint8_t[size];
    if (buffer == nullptr) {
      return false;
    }

    _file.seek(_start);
    _file.read(buffer, size);

    _image = new Image(Image::IMAGE_JPEG, buffer, size, true);
    delete[] buffer;
    
    if (_image == nullptr) {
      return false;
    }

    if (!_image->isLoaded()) {
      delete _image;
      _image = nullptr;
      return false;
    }
    
    return true;
  }
}


void Video::mjpeg_read_entry_point(void* param) {
  auto pthis = (Video*)param;
  pthis->mjpeg_read();

  vTaskDelete(nullptr);
}


void Video::mjpeg_decode_entry_point(void* param) {
  auto pthis = (Video*)param;
  pthis->mjpeg_decode();

  vTaskDelete(nullptr);
}


void Video::mjpeg_read() {
  while (_buffering && !_buffering_fail) {
    if (_file.available()) {
      int index = find_free_buffer();
      if (index >= 0) {
        _buffer[index].start = _file.position();
        _buffer[index].read = _file.read(_buffer[index].ptr, _buffer[index].size);
        _buffer[index].last = !_file.available();
        _buffer[index].valid = true;
      }
    }
    else if (_buffering_loop) {
      int index = find_free_buffer();
      if (index >= 0) {
        _file.seek(0);
      }
    }

    delay(1);
  }

  _mjpeg_read_end = true;
}


void Video::mjpeg_decode() {
  while (_buffering && !_buffering_fail) {
    mjpeg_decode2();

    delay(1);
  }

  _mjpeg_decode_end = true;
}


void Video::mjpeg_decode2() {
  if (_decoded.size() >= MJPEG_LOAD_FRAMES) {
    return;
  }

  if (!find_range()) {
    if (has_last()) {
      _start = _end = 0;
      flush_buffer(true);
    }
    return;
  }

  flush_buffer(false);
  
  if (_start >= _end) {
    return;
  }

  int index1 = find_buffer(_start);
  if (index1 < 0) {
    return;
  }

  int index2 = find_buffer(_end);
  if (index2 < 0) {
    return;
  }

  size_t size = _end - _start;
  
  Image* image;
  if (index1 == index2) {
    // continuous data
    image = new Image(Image::IMAGE_JPEG, _buffer[index1].ptr + _start - _buffer[index1].start, size, true);
  }
  else {
    // fragmented data
    uint8_t* buffer = (uint8_t*)malloc_auto(size);
    size_t size1 = _buffer[index1].size - (_start - _buffer[index1].start);
    size_t size2 = size - size1;
    memcpy(buffer, _buffer[index1].ptr + _start - _buffer[index1].start, size1);
    memcpy(buffer + size1, _buffer[index2].ptr, size2);
    image = new Image(Image::IMAGE_JPEG, buffer, size, true);
    free_auto(buffer);
  }

  if (image == nullptr) {
    _buffering_fail = true;
    return;
  }

  if (!image->isLoaded()) {
    // _buffering_fail = true;
    TF_LOGW(TAG, "fail %d %d", _start, _end);
    delete image;
  }
  else {
    if (xSemaphoreTake(_interlock, portMAX_DELAY)) {
      _decoded.push_back(image);
      xSemaphoreGive(_interlock);
    }
  }
}


int Video::find_buffer(size_t pos) {
  for (int i = 0; i < MJPEG_BUFFERS; i++) {
    if (_buffer[i].ptr == nullptr || !_buffer[i].valid) {
      continue;
    }

    if (pos >= _buffer[i].start && pos < _buffer[i].start + _buffer[i].size) {
      return i;
    }
  }

  return -1;
}


int Video::find_free_buffer() {
  for (int i = 0; i < MJPEG_BUFFERS; i++) {
    if (_buffer[i].ptr == nullptr || _buffer[i].valid) {
      continue;
    }
    
    return i;
  }

  return -1;
}


bool Video::find_range() {
  int step = 0;
  bool find_header = false;
  int start = -1;
  int end = -1;

  size_t file_size = _file.size();

  for (size_t i = _start; i < file_size; i++) {
    int index = find_buffer(i);
    if (index < 0) {
      return false;
    }

    uint8_t read = _buffer[index].ptr[i - _buffer[index].start];

    switch (step) {
    case 0: // find ff
      if (read == 0xff) {
        start = i;
        step = 1;
      }
      break;
    case 1: // find d8
      if (read == 0xd8) {
        find_header = true;
        step = 2;
      }
      else if (read == 0xff) {
        start = i;
      }
      else {
        step = 0;
      }
      break;
    case 2: // find next ff
      if (read == 0xff) {
        end = i;
        step = 3;
      }
      break;
    case 3: // find next d8
      if (read == 0xd8) {
        _start = start;
        _end = end;
        return true;
      }
      else if (read == 0xff) {
        end = i;
      }
      else {
        step = 2;
      }
      break;
    }
  }

  if (find_header) {
    _start = start;
    _end = end;
    return true;
  }

  return false;
}


bool Video::find_range_direct() {
  int step = 0;
  bool find_header = false;
  int start = -1;
  int end = -1;

  const size_t BLOCK_SIZE = CONFIG_WL_SECTOR_SIZE;
  uint8_t* block = new uint8_t[BLOCK_SIZE];

  while (_file.available()) {
    size_t block_pos = _file.position();
    size_t bytes_read = _file.read(block, BLOCK_SIZE);
    
    for (size_t i = 0; i < bytes_read; i++) {
      size_t pos = block_pos + i;
      uint8_t read = block[i];

      switch (step) {
      case 0: // find ff
        if (read == 0xff) {
          start = pos;
          step = 1;
        }
        break;
      case 1: // find d8
        if (read == 0xd8) {
          find_header = true;
          step = 2;
        }
        else if (read == 0xff) {
          start = pos;
        }
        else {
          step = 0;
        }
        break;
      case 2: // find next ff
        if (read == 0xff) {
          end = pos;
          step = 3;
        }
        break;
      case 3: // find next d8
        if (read == 0xd8) {
          delete[] block;
          _start = start;
          _end = end;
          return true;
        }
        else if (read == 0xff) {
          end = pos;
        }
        else {
          step = 2;
        }
        break;
      }
    }
  }
  
  delete[] block;

  if (find_header) {
    _start = start;
    _end = end;
    return true;
  }

  return false;
}


void Video::flush_buffer(bool init) {
  if (init) {
    for (int i = 0; i < MJPEG_BUFFERS; i++) {
      if (!_buffer[i].valid) {
        continue;
      }

      if (_buffer[i].last || _buffer[i].start > (MJPEG_BUFFER_SIZE * (MJPEG_BUFFERS - 2))) {
        _buffer[i].valid = false;
      }
    }
  }
  else {
    for (int i = 0; i < MJPEG_BUFFERS; i++) {
      if (!_buffer[i].valid) {
        continue;
      }

      if (has_last()) {
        if (_buffer[i].start > (MJPEG_BUFFER_SIZE * (MJPEG_BUFFERS - 2))
        && _start >= _buffer[i].start + _buffer[i].size) {
          _buffer[i].valid = false;
        }
      }
      else {
        if (_start >= _buffer[i].start + _buffer[i].size) {
          _buffer[i].valid = false;
        }
      }
    }
  }
}


bool Video::has_last() {
  for (int i = 0; i < MJPEG_BUFFERS; i++) {
    if (_buffer[i].ptr == nullptr || !_buffer[i].valid) {
      continue;
    }

    if (_buffer[i].last) {
      return true;
    }
  }

  return false;
}


};
