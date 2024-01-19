#pragma once
#include "effect_base.h"
#include "effect_parser.h"

#include "lib/logger.h"


namespace toaster {

class EffectScript : public FixedEffect {
public:
  EffectScript(const char* name) : FixedEffect(name) {
  }
  
  virtual void init(Display& display) {
    Effect::init(display);
    
    for (auto& it : _parser.getImages()) {
      auto image = display.load_png(it.c_str());
      _images.push_back(image);
    }

    _parser.releaseImages();
  }

  virtual void process(Display& display) {
    if (_error) {
      return;
    }

    const auto& sequence = _parser.getSequences();

    if (sequence.empty()) {
      TF_LOG("EffectScript", "Sequence does not exist.");
      _error = true;
      return;
    }

    const auto& current = sequence[_step];

    switch (current.method) {
    case Effect::SM_NONE:
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270:
      if (current.image >= _images.size()) {
        TF_LOG("EffectScript", "image index out of range. (%d / %d)", current.image, _images.size());
        _error = true;
        return;
      }

      if (current.color == Effect::PC_ORIGINAL) {
        display.draw_png(_images[current.image], (DRAW_MODE)current.mode, current.offset_x, current.offset_y, current.method - Effect::SM_DRAW);
      }
      else {
        display.draw_png_newcolor(_images[current.image], _colorFunc, current.color, (DRAW_MODE)current.mode, current.offset_x, current.offset_y, current.method - Effect::SM_DRAW);
      }
      break;
    }

    if (sequence.size() >= 2 && timeout(current.display_time)) {
      if (_step >= sequence.size() - 1) {
        setStep(0);
      }
      else {
        setStep(_step + 1);
      }
    }
  }

  virtual void release(Display& display) {
    for (auto& it : _images) {
      display.unload_png(it);
    }
    _images.clear();

    _parser.release();
    
    Effect::release(display);
  }

  virtual const char* getName() const {
    return _script_name.c_str();
  }

  virtual bool isScript() const {
    return true;
  }

  virtual bool loadScript(const char* name) {
    _script_name = name;

    char filename[256] = {0,};
    sprintf(filename, "/script/%s.yaml", name);

    bool result = _parser.open(filename);
    if (result == false) {
      TF_LOG("EffectScript", "script %s parse failed (%s)", filename, _parser.getLastError());
    }

    return result;
  }


protected:
  bool _error{false};
  std::vector<upng_t*> _images;
  EffectParser _parser;
  std::string _script_name;

};

};
