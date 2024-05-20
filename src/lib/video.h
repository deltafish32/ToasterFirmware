#pragma once
#include "image.h"


namespace toaster {


class Video {
public:
  Video(const char* path, uint32_t mjpeg_fps = 0);
  virtual ~Video();

public:
  void release();
  bool isLoaded();

  const Image* getImage() const;
  bool nextFrame(bool loop, bool init = false);
  bool firstFrame() {
    return nextFrame(false, true);
  }
  
  size_t getWidth() const {
    return (_image != nullptr) ? _image->getWidth() : 0;
  }

  size_t getHeight() const {
    return (_image != nullptr) ? _image->getHeight() : 0;
  }

  timer_pf_t getPF() const {
    return _pf;
  }

  timer_ms_t getPF_ms() const {
    return (_pf.type == PF_FREQUENCY) ? (1000 / _pf.frequency) : (_pf.type == PF_PERIOD) ? _pf.period_ms : 0;
  }

public:
  enum VIDEO_TYPE {
    VIDEO_NONE = 0,
    VIDEO_GIF,
    VIDEO_MJPEG,

    VIDEO_LEGACY = VIDEO_NONE,
  };

protected:
  File _file;
  Image* _image{nullptr};
  
  uint32_t _frame{0};
  uint8_t _type{VIDEO_NONE}; // VIDEO_TYPE
  uint8_t _spare0{0};
  uint8_t _spare1{0};
  uint8_t _spare2{0};
  timer_pf_t _pf{PF_NONE, };

  // gif only
  void* _gif{nullptr};
  std::string _path;

  // mjpeg only
  size_t _start{0};
  size_t _end{0};
  
protected:
  bool load_gif(const char* path);
  bool next_gif(bool loop, bool init = false);
  bool load_gif_frame();
  bool load_mjpeg(const char* path, uint32_t fps);
  bool next_mjpeg(bool loop, bool init = false);
  bool load_mjpeg_frame();
};


};
