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

  virtual bool isSameEffect(const char* name, const char* base_path) const {
    return strcasecmp(getName(), name) == 0
        && _requested_base_path == ((base_path != nullptr) ? base_path : "");
  }

  virtual bool isScript() const {
    return true;
  }

  virtual bool isVideoMode() const {
    return (_parser.getType() == ST_VIDEO) || _video_flag;
  }

  virtual timer_pf_t getVideoPF() const {
    if (_parser.getType() == ST_VIDEO && _video_legacy) {
      timer_pf_t pf{PF_NONE, };
      if (_parser.getVideoInfo().fps > 0) {
        pf.type = PF_FREQUENCY;
        pf.frequency = _parser.getVideoInfo().fps;
      }
      return pf;
    }

    if (_video_index >= _assets.size() || _assets[_video_index] == nullptr || _assets[_video_index]->getVideo() == nullptr) {
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
  std::string _requested_base_path;
  bool _from_sd{false};
  bool _boop{false};

protected:
  Asset* _video_legacy_frame{nullptr};
  bool _video_legacy{false};
  int _video_offset_x{0};
  int _video_offset_y{0};
  bool _video_flag{false};
  size_t _video_index{0};
  int _video_color{0};

protected:
  uint32_t getLegacyVideoPF_ms() const {
    uint32_t fps = _parser.getVideoInfo().fps;
    return (fps > 0) ? (1000 / fps) : 0;
  }

  std::string makeFileName(const char* filename);
  bool initSequence(Display& display);
  bool initVideo(Display& display);
  void processSequence(Display& display, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence);
  void processVideo(Display& display);
  size_t releaseSomeImages(Display& display, int current_sequence, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence, size_t to_release = 1);
  size_t loadNextImages(Display& display, int current_sequence, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence, size_t to_load = 1);

};

};
