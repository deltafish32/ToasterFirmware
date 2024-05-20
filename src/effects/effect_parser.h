#pragma once
#include "lib/yaml.h"


namespace toaster {


enum {
  ST_SEQUENCE = 0,
  ST_VIDEO,
};


typedef struct _DRAW_SEQUENCE {
  uint8_t method{0}; // 0: none, 1: draw, 2: draw_90, 3: draw_180, 4: draw_270
  uint8_t image{0};
  uint8_t color{0}; // 0: eyes, 1: nose, 2: mouth, 3: side
  uint8_t mode{0}; // 0: single, 1: copy, 2: mirror, 3: mirror_only_offset
  int32_t offset_x{0};
  int32_t offset_y{0};
  uint32_t display_time{0}; // 1000 = 1s
} DRAW_SEQUENCE;


typedef struct _VIDEO_INFO {
  std::string path;
  int start{0};
  int end{0};
  uint32_t fps{0};
  uint8_t mode{0}; // 0: single, 1: copy, 2: mirror, 3: mirror_only_offset
  uint8_t offset_mode{0}; // 0: manual, 1: auto
  bool loop{0};
  int32_t offset_x{0};
  int32_t offset_y{0};
} VIDEO_INFO;


class EffectParser {
public:
  EffectParser() {}

  bool open(const char* filename);
  
  void release() {
    _images.clear();
    _sequences.clear();
    _boop_sequences.clear();
  }

  uint32_t getVersion() const {
    return _version;
  }

  const std::vector<std::string>& getImages() const {
    return _images;
  }

  const std::vector<DRAW_SEQUENCE>& getSequences() const {
    return _sequences;
  }

  const std::vector<DRAW_SEQUENCE>& getBoopSequences() const {
    return _boop_sequences;
  }

  uint8_t getType() const {
    return _type;
  }

  const VIDEO_INFO& getVideoInfo() const {
    return _video;
  }

public:
  bool isError() const {
    return !_error.empty();
  }

  const char* getLastError() const {
    return _error.c_str();
  }

protected:
  enum {
    PD_SUCCESS = 0,
    PD_FAIL,
    PD_SKIP,
  };

  static uint8_t parseDrawSequence(const std::string& str, DRAW_SEQUENCE& ds, std::string& error);

protected:
  std::string _error;
  uint32_t _version{0};
  uint8_t _type{0};
  std::vector<std::string> _images;
  std::vector<DRAW_SEQUENCE> _sequences;
  std::vector<DRAW_SEQUENCE> _boop_sequences;
  VIDEO_INFO _video;

};

};
