#pragma once
#include "effect_base.h"
#include "effect_parser.h"


namespace toaster {

class EffectScript : public FixedEffect {
public:
  EffectScript(const char* name);
  
  virtual void init(Display& display);
  virtual void process(Display& display);
  virtual void release(Display& display);

  virtual const char* getName() const {
    return _script_name.c_str();
  }

  virtual bool isScript() const {
    return true;
  }

  virtual bool isVideoMode() const {
    return (_parser.getType() == ST_VIDEO);
  }

  virtual uint32_t getVideoFPS() const {
    return _parser.getVideoInfo().fps;
  }

  virtual void restart() {
    if (_parser.getType() == ST_VIDEO) {
      setStep(_parser.getVideoInfo().start);
    }
  }

  virtual bool loadScript(const char* name);

protected:
  bool _error{false};
  std::vector<upng_t*> _images;
  EffectParser _parser;
  std::string _script_name;
  bool _boop{false};

protected:
  upng_t* _recent_frame{nullptr};
  int _video_offset_x{0};
  int _video_offset_y{0};

protected:
  size_t releaseSomeImages(Display& display, int current_image, const std::vector<DRAW_SEQUENCE>& sequence, size_t to_release = 1);
  size_t loadNextImages(Display& display, int current_image, const std::vector<DRAW_SEQUENCE>& sequence, size_t to_load = 1);

};

};
