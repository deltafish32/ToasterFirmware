#pragma once
#include "lib/yaml.h"


namespace toaster {

typedef struct _DRAW_SEQUENCE {
  uint8_t method{0}; // 0: none, 1: draw, 2: draw_90, 3: draw_180, 4: draw_270
  uint8_t image{0};
  uint8_t color{0}; // 0: eyes, 1: nose, 2: mouth, 3: side
  uint8_t mode{0}; // 0: single, 1: copy, 2: mirror, 3: mirror_only_offset
  int32_t offset_x{0};
  int32_t offset_y{0};
  uint32_t display_time{0}; // 1000 = 1s
} DRAW_SEQUENCE;


class EffectParser {
public:
  EffectParser() {}

  bool open(const char* filename);
  
  void release() {
    _images.clear();
    _sequences.clear();
  }

  void releaseImages() {
    _images.clear();
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

public:
  bool isError() const {
    return !_error.empty();
  }

  const char* getLastError() const {
    return _error.c_str();
  }

protected:
  std::string _error;
  uint32_t _version{0};
  std::vector<std::string> _images;
  std::vector<DRAW_SEQUENCE> _sequences;

};

};
