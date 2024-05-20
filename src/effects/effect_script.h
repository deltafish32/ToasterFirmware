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
    return (_parser.getType() == ST_VIDEO) || _video_flag;
  }

  virtual timer_pf_t getVideoPF() const {
    if (_video_index >= _assets.size() || _assets[_video_index] == nullptr) {
      return {PF_NONE, };
    }

    return _assets[_video_index]->getVideo()->getPF();
  }

  virtual void restart() {
    switch (_parser.getType()) {
    case ST_SEQUENCE:
      restartTimer();
      break;
    case ST_VIDEO:
      setStep(_parser.getVideoInfo().start);
      break;
    }
  }

  virtual bool loadScript(const char* name, const char* base_path);

protected:
  bool _error{false};
  std::vector<Asset *> _assets;
  EffectParser _parser;
  std::string _script_name;
  std::string _base_path;
  bool _boop{false};

protected:
  Asset* _video_legacy_frame{nullptr};
  bool _video_legacy{false};
  int _video_offset_x{0};
  int _video_offset_y{0};
  bool _video_flag{false};
  size_t _video_index{0};

protected:
  std::string makeFileName(const char* filename);
  bool initSequence(Display& display);
  bool initVideo(Display& display);
  void processSequence(Display& display, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence);
  void processVideo(Display& display);
  size_t releaseSomeImages(Display& display, int current_sequence, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence, size_t to_release = 1);
  size_t loadNextImages(Display& display, int current_sequence, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence, size_t to_load = 1);

};

};
