#pragma once
#include "image.h"
#include <list>


// At least 3 buffers are required
#define MJPEG_BUFFERS     3


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
  size_t _start{0};
  size_t _end{0};

  typedef struct _MJPEG_BUFFER {
    uint8_t* ptr{nullptr};
    size_t start{0};
    size_t size{0};
    size_t read{0};
    bool valid{false};
    bool last{false};
  } MJPEG_BUFFER;
  MJPEG_BUFFER _buffer[MJPEG_BUFFERS];
  bool _buffering{false};
  bool _buffering_loop{false};
  bool _mjpeg_read_end{false};
  bool _mjpeg_decode_end{false};
  bool _buffering_fail{false};

  SemaphoreHandle_t _interlock{nullptr};
  std::list<Image*> _decoded;

  
protected:
  bool load_gif(const char* path, bool from_sd, bool loop);
  bool next_gif(bool loop, bool init = false);
  bool load_gif_frame();
  bool load_mjpeg(const char* path, bool from_sd, bool loop, uint32_t fps);
  bool next_mjpeg(bool loop, bool init = false);
  bool load_mjpeg_frame();
  static void mjpeg_read_entry_point(void* param);
  static void mjpeg_decode_entry_point(void* param);
  void mjpeg_read();
  void mjpeg_decode();
  void mjpeg_decode2();
  int find_buffer(size_t pos);
  int find_free_buffer();
  bool find_range();
  bool find_range_direct();
  void flush_buffer(bool init);
  bool has_last();
};


};
