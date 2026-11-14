#include "config/configure.h"
#include <new>
#include <algorithm>
#include "video.h"
#include "logger.h"
#include <AnimatedGIF.h>
#include "ram.h"


#ifdef USE_SD
#include "SD.h"
#endif


namespace toaster {

static const char* TAG = "Video";

static const size_t MJPEG_WINDOW_SIZE = 64 * 1024;

enum {
  FRAME_FOUND = 0,
  FRAME_SKIP,
  FRAME_END,
};


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
  stop_mjpeg_task();

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
  File* file_ptr = new (std::nothrow) File;
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

  if (pDraw->ucHasTransparency && pDraw->ucDisposalMethod != 2) {
    auto ptr_buffer = buffer + (((pDraw->iY + pDraw->y) * width + (pDraw->iX)) * 3);
    for (int x = 0; x < pDraw->iWidth; x++, ptr_buffer += 3) {
      uint8_t color = pDraw->pPixels[x];
      if (color == ucTransparent) {
        continue;
      }
      const auto& pal = palette[color];
      ptr_buffer[0] = pal.r;
      ptr_buffer[1] = pal.g;
      ptr_buffer[2] = pal.b;
    }
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
  bool exists = false;
  if (from_sd) {
#ifdef USE_SD
    exists = SD.exists(path);
#endif
  }
  else {
    exists = FFat.exists(path);
  }

  if (!exists) {
    TF_LOGE(TAG, "gif file open failed (sd: %d, %s).", from_sd, path);
    return false;
  }

  auto gif = new (std::nothrow) AnimatedGIF;
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
  _image = new (std::nothrow) Image(width, height, 3, 0);
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

  if (psramFound()) {
    _interlock = xSemaphoreCreateMutex();
    if (_interlock == nullptr) {
      TF_LOGE(TAG, "video mutex creation failed (%s).", path);
      release();
      return false;
    }

    _mjpeg_run = true;
    _mjpeg_task_end = false;
    _mjpeg_fail = false;

    auto core_result = xTaskCreatePinnedToCore(mjpeg_entry_point, "mjpeg", 8192, this, 1, nullptr, PRO_CPU_NUM);
    if (core_result != pdPASS) {
      TF_LOGE(TAG, "xTaskCreatePinnedToCore failed (%d).", core_result);
      _mjpeg_run = false;
      _mjpeg_task_end = true;
      release();
      return false;
    }
  }

  if (!next_mjpeg(false, true)) {
    TF_LOGE(TAG, "video mjpeg first frame load failed. (%s)", path);
    release();
    return false;
  }

  _pf.type = PF_FREQUENCY;
  _pf.frequency = fps;

  return true;
}


bool Video::next_mjpeg(bool loop, bool init) {
  return psramFound() ? next_mjpeg_decoded(loop, init) : next_mjpeg_direct(loop, init);
}


bool Video::next_mjpeg_decoded(bool loop, bool init) {
  static const timer_ms_t FIRST_FRAME_TIMEOUT_MS = 1000;

  timer_ms_t tick = Timer::get_millis();

  while (true) {
    if (_mjpeg_fail) {
      return false;
    }

    bool popped = false;
    Image* image = nullptr;

    if (xSemaphoreTake(_interlock, portMAX_DELAY)) {
      if (!_decoded.empty()) {
        image = _decoded.front();
        _decoded.pop_front();
        popped = true;
      }
      xSemaphoreGive(_interlock);
    }

    if (popped) {
      if (image == nullptr) {
        if (loop || init) {
          continue;
        }
        return false;
      }

      if (_image != nullptr) {
        delete _image;
      }
      _image = image;
      _frame = init ? 0 : _frame + 1;
      return true;
    }

    // keep the current frame rather than stall rendering while the decoder catches up
    if (!init) {
      return true;
    }

    if (Timer::get_millis() - tick >= FIRST_FRAME_TIMEOUT_MS) {
      return false;
    }

    delay(1);
  }
}


bool Video::next_mjpeg_direct(bool loop, bool init) {
  if (init) {
    _file.seek(0);
  }

  if (!find_frame()) {
    _file.seek(0);
    if ((!loop && !init) || !find_frame()) {
      return false;
    }
    init = true;
  }

  Image* image = read_frame();
  if (image == nullptr) {
    return false;
  }

  if (_image != nullptr) {
    delete _image;
  }
  _image = image;
  _frame = init ? 0 : _frame + 1;

  return true;
}


void Video::stop_mjpeg_task() {
  static const timer_ms_t STOP_TIMEOUT_MS = 2000;

  if (_mjpeg_task_end) {
    return;
  }

  _mjpeg_run = false;

  timer_ms_t tick = Timer::get_millis();
  while (!_mjpeg_task_end) {
    if (Timer::get_millis() - tick >= STOP_TIMEOUT_MS) {
      TF_LOGE(TAG, "mjpeg decode task did not stop.");
      break;
    }
    delay(1);
  }
}


void Video::mjpeg_entry_point(void* param) {
  auto pthis = (Video*)param;
  pthis->mjpeg_decode();

  vTaskDelete(nullptr);
}


void Video::mjpeg_decode() {
  static const size_t QUEUE_FRAMES = 8;

  _window = (uint8_t*)malloc_auto(MJPEG_WINDOW_SIZE);
  if (_window == nullptr) {
    TF_LOGE(TAG, "mjpeg read buffer allocation failed.");
    _mjpeg_fail = true;
    _mjpeg_task_end = true;
    return;
  }
  window_rewind();

  bool has_frame = false;

  while (_mjpeg_run) {
    size_t queued = QUEUE_FRAMES;
    if (xSemaphoreTake(_interlock, portMAX_DELAY)) {
      queued = _decoded.size();
      xSemaphoreGive(_interlock);
    }

    if (queued >= QUEUE_FRAMES) {
      delay(1);
      continue;
    }

    const uint8_t* data = nullptr;
    size_t size = 0;
    uint8_t result = window_next_frame(data, size);
    if (result == FRAME_SKIP) {
      continue;
    }

    Image* image = nullptr;

    if (result == FRAME_FOUND) {
      image = new (std::nothrow) Image(Image::IMAGE_JPEG, data, size, true);
      if (image == nullptr || !image->isLoaded()) {
        delete image;
        TF_LOGW(TAG, "mjpeg frame decode failed (%u bytes).", (unsigned)size);
        continue;
      }
      has_frame = true;
    }
    else {
      if (!has_frame) {
        TF_LOGE(TAG, "mjpeg has no frames.");
        _mjpeg_fail = true;
        break;
      }
      has_frame = false;
      window_rewind();
    }

    if (xSemaphoreTake(_interlock, portMAX_DELAY)) {
      _decoded.push_back(image);
      xSemaphoreGive(_interlock);
    }

    delay(1);
  }

  free_auto(_window);
  _window = nullptr;

  _mjpeg_task_end = true;
}


void Video::window_rewind() {
  _file.seek(0);
  _window_pos = 0;
  _window_len = 0;
  _cursor = 0;
}


// Moves the data from keep_from to the end of the window to the front, then reads more of the file after it.
bool Video::window_fill(size_t keep_from) {
  size_t window_end = _window_pos + _window_len;
  keep_from = std::min(std::max(keep_from, _window_pos), window_end);

  size_t keep = window_end - keep_from;
  if (keep > 0 && keep_from > _window_pos) {
    memmove(_window, _window + (keep_from - _window_pos), keep);
  }
  _window_pos = keep_from;
  _window_len = keep;

  size_t bytes_read = _file.read(_window + keep, MJPEG_WINDOW_SIZE - keep);
  _window_len += bytes_read;

  return (bytes_read > 0);
}


bool Video::window_find_soi(size_t from, size_t& found) {
  size_t window_end = _window_pos + _window_len;

  for (size_t i = std::max(from, _window_pos); i + 1 < window_end; i++) {
    const uint8_t* ptr = _window + (i - _window_pos);
    if (ptr[0] == 0xff && ptr[1] == 0xd8) {
      found = i;
      return true;
    }
  }

  return false;
}


uint8_t Video::window_next_frame(const uint8_t*& data, size_t& size) {
  size_t start = 0;
  size_t from = _cursor;
  while (!window_find_soi(from, start)) {
    size_t window_end = _window_pos + _window_len;
    from = (window_end > 0) ? (window_end - 1) : 0;
    if (!window_fill(from)) {
      return FRAME_END;
    }
  }

  size_t end = 0;
  from = start + 2;
  while (!window_find_soi(from, end)) {
    size_t window_end = _window_pos + _window_len;

    if (start == _window_pos && _window_len == MJPEG_WINDOW_SIZE) {
      TF_LOGW(TAG, "mjpeg frame too large (> %u bytes).", (unsigned)MJPEG_WINDOW_SIZE);
      _cursor = window_end - 1;
      return FRAME_SKIP;
    }

    from = std::max(window_end - 1, start + 2);
    if (!window_fill(start)) {
      end = window_end;
      break;
    }
  }

  data = _window + (start - _window_pos);
  size = end - start;
  _cursor = end;

  return FRAME_FOUND;
}


// A frame starts at an SOI marker (FF D8) and ends where the next one starts, or at the end of the file.
bool Video::find_frame() {
  const size_t BLOCK_SIZE = CONFIG_WL_SECTOR_SIZE;
  uint8_t* block = (uint8_t*)malloc_auto(BLOCK_SIZE);
  if (block == nullptr) {
    return false;
  }

  bool found_start = false;
  size_t start = 0;
  bool has_prev = false;
  uint8_t prev = 0;

  while (_file.available()) {
    size_t block_pos = _file.position();
    size_t bytes_read = _file.read(block, BLOCK_SIZE);
    if (bytes_read == 0) {
      break;
    }

    for (size_t i = 0; i < bytes_read; i++) {
      uint8_t read = block[i];

      if (has_prev && prev == 0xff && read == 0xd8) {
        size_t marker = block_pos + i - 1;

        if (!found_start) {
          found_start = true;
          start = marker;
        }
        else {
          free_auto(block);
          _start = start;
          _end = marker;
          return true;
        }
      }

      prev = read;
      has_prev = true;
    }
  }

  free_auto(block);

  if (found_start) {
    _start = start;
    _end = _file.size();
    return true;
  }

  return false;
}


Image* Video::read_frame() {
  static const size_t FRAME_SIZE_MAX = 64 * 1024;

  size_t size = _end - _start;
  if (size == 0 || size > FRAME_SIZE_MAX) {
    TF_LOGW(TAG, "mjpeg frame size out of range (%u).", (unsigned)size);
    _file.seek(_end);
    return nullptr;
  }

  uint8_t* buffer = (uint8_t*)malloc_auto(size);
  if (buffer == nullptr) {
    TF_LOGE(TAG, "mjpeg frame memory allocation failed (%u).", (unsigned)size);
    _file.seek(_end);
    return nullptr;
  }

  _file.seek(_start);
  size_t bytes_read = _file.read(buffer, size);

  Image* image = nullptr;
  if (bytes_read == size) {
    image = new (std::nothrow) Image(Image::IMAGE_JPEG, buffer, size, true);
    if (image != nullptr && !image->isLoaded()) {
      delete image;
      image = nullptr;
    }
  }
  free_auto(buffer);

  return image;
}


};
