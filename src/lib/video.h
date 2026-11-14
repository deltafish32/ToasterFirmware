#pragma once
#include "image.h"
#include <list>


namespace toaster {


class Video {
public:
  Video(const char* path, bool from_sd, bool loop, uint32_t mjpeg_fps = 0);
  virtual ~Video();

public:
  void release();
  bool isLoaded() const;

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
    return (_pf.type == PF_FREQUENCY) ? (1000 / ((_pf.frequency > 0) ? _pf.frequency : 1)) : (_pf.type == PF_PERIOD) ? _pf.period_ms : 0;
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
  // With PSRAM, the decode task owns _file and the read window and passes frames through
  // _decoded. A nullptr entry marks the end of the video, after which the first frame follows.
  size_t _start{0};
  size_t _end{0};

  uint8_t* _window{nullptr};
  size_t _window_pos{0};
  size_t _window_len{0};
  size_t _cursor{0};

  volatile bool _mjpeg_run{false};
  volatile bool _mjpeg_task_end{true};
  volatile bool _mjpeg_fail{false};
  volatile bool _mjpeg_rewind{false};
  TaskHandle_t _mjpeg_task{nullptr};

  SemaphoreHandle_t _interlock{nullptr};
  std::list<Image*> _decoded;


protected:
  bool load_gif(const char* path, bool from_sd, bool loop);
  bool next_gif(bool loop, bool init = false);
  bool load_gif_frame();
  bool load_mjpeg(const char* path, bool from_sd, bool loop, uint32_t fps);
  bool next_mjpeg(bool loop, bool init = false);
  bool next_mjpeg_decoded(bool loop, bool init);
  bool next_mjpeg_direct(bool loop, bool init);
  void stop_mjpeg_task();
  static void mjpeg_entry_point(void* param);
  void mjpeg_decode();
  void window_rewind();
  bool window_fill(size_t keep_from);
  bool window_find_soi(size_t from, size_t& found);
  uint8_t window_next_frame(const uint8_t*& data, size_t& size);
  bool find_frame();
  Image* read_frame();
};


};
