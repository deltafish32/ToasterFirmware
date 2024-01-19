#pragma once
#include "config/configure.h"

#include "hal/display/display.h"
#include "lib/random.h"
#include "lib/linear_calibrate.h"

#include "protogen_color_mode.h"


namespace toaster {

class Effect : protected Random {
public:
  Effect(const char* name) {
    _name = name;
    _effects.push_back(this);
  }

  virtual ~Effect() {
    for (auto it = _effects.begin(); it != _effects.end(); ) {
      if ((*it) == this) {
        it = _effects.erase(it);
      }
      else {
        ++it;
      }
    }
  }


public:
  virtual void init(Display& display) {
    setStep(0);
  }
  
  virtual void process(Display& display) = 0;
  virtual void release(Display& display) {
  }
  
  const char* getName() const {
    return _name;
  }

  virtual bool isBlank() const {
    return false;
  }

  virtual bool isScript() const {
    return false;
  }

  virtual bool loadScript(const char* name) {
    return false;
  }


public:
  static bool getStaticMode() {
    return _staticMode;
  }

  static void setStaticMode(bool mode) {
    if (_staticMode != mode) {
      _staticMode = mode;

      if (_staticMode) {
        _staticModeTick = millis();
      }
      else {
        uint32_t tickPaused = millis() - _staticModeTick;
        for (auto& it : _effects) {
          it->_tick += tickPaused;
        }
        
        _colorTick += millis() - _staticModeTick;
      }
    }
  }

  static PROTOGEN_COLOR_MODE getColorMode() {
    return _colorMode;

  }

  static void setColorMode(PROTOGEN_COLOR_MODE mode) {
    _colorMode = mode;

    if (mode == PCM_ORIGINAL) {
      _colorFunc = color_func_original;
    }
    if (mode == PCM_PERSONAL) {
      _colorFunc = color_func_personal;
    }
    if (mode == PCM_RAINBOW_SINGLE) {
      _colorFunc = color_func_rainbow_single;
    }
    if (mode == PCM_RAINBOW) {
      _colorFunc = color_func_rainbow;
    }
    if (mode == PCM_GRADATION) {
      _colorFunc = color_func_gradation;
    }

    setDirty();
  }

  static void setPersonalColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    _personalColor[index][0] = r;
    _personalColor[index][1] = g;
    _personalColor[index][2] = b;
  }

  static void setPersonalColor(uint8_t index, uint32_t rgb) {
    setPersonalColor(index, (rgb & 0xff), ((rgb >> 8) & 0xff), ((rgb >> 16) & 0xff));
  }

  static void setRainbowSingleSpeed(uint32_t speed_ms) {
    _rainbowSingleSpeed_ms = speed_ms;

    setDirty();
  }

  static void setRainbowSpeed(uint32_t speed_ms, uint32_t transition_pixels) {
    _rainbowSpeed_ms = speed_ms;
    _rainbowTransitionPixels = transition_pixels;

    setDirty();
  }

  static void setGradationPoint(float p, uint8_t r, uint8_t g, uint8_t b) {
    _gradation_r.pushDataPoint({p, r});
    _gradation_g.pushDataPoint({p, g});
    _gradation_b.pushDataPoint({p, b});

    setDirty();
  }

  static void setDirty() {
    _rainbow_single_dirty = true;
    _rainbow_dirty = true;
    _gradation_dirty = true;
  }

  static Effect* find(const char* name, uint8_t script_index = 0) {
    if (*name == 0) {
      return nullptr;
    } 
    
    auto result = findFromHardcoded(name);
    if (result != nullptr) {
      return result;
    }

    return findFromScript(name, script_index);
  }

  static Effect* findFromHardcoded(const char* name) {
    for (auto it = _effects.begin(); it != _effects.end(); ++it) {
      if (strcasecmp((*it)->_name, name) == 0) {
        return *it;
      }
    }

    return nullptr;
  }

  static Effect* findFromScript(const char* name, uint8_t script_index);


protected:
  static std::vector<Effect*> _effects;


protected:
  const char* _name{0,};
  int _step{0};
  uint32_t _tick{0};

  void setStep(int step) {
    _step = step;
    _tick = millis();

    if (_staticMode) {
      _staticModeTick = _tick;
    }
  }

  bool timeout(uint32_t t) {
    if (_staticMode) {
      return false;
    }

    return ((millis() - _tick) >= t);
  }


public:
  enum {
    SM_NONE = 0,
    SM_DRAW,
    SM_DRAW_90,
    SM_DRAW_180,
    SM_DRAW_270,
  };

  enum {
    PC_ORIGINAL = 0,
    PC_EYES,
    PC_NOSE,
    PC_MOUTH,
    PC_SIDE,
  };

  enum {
    DM_SINGLE = 0,
    DM_COPY,
    DM_MIRROR,
    DM_MIRROR_ONLY_OFFSET,
  };


protected:
  static bool _staticMode;
  static uint32_t _staticModeTick;
  static uint32_t _colorTick;
  static PROTOGEN_COLOR_MODE _colorMode;
  static COLOR_FUNC _colorFunc;
  static uint8_t _personalColor[4][3];
  static uint32_t _rainbowSingleSpeed_ms;
  static uint32_t _rainbowSpeed_ms;
  static uint32_t _rainbowTransitionPixels;
  static LinearCalibrate<float, uint32_t> _gradation_r;
  static LinearCalibrate<float, uint32_t> _gradation_g;
  static LinearCalibrate<float, uint32_t> _gradation_b;
  static bool _rainbow_single_dirty;
  static uint8_t _rainbow_single_red;
  static uint8_t _rainbow_single_green;
  static uint8_t _rainbow_single_blue;
  static bool _rainbow_dirty;
  static uint8_t _rainbow_red[HUB75_PANEL_RES_X];
  static uint8_t _rainbow_green[HUB75_PANEL_RES_X];
  static uint8_t _rainbow_blue[HUB75_PANEL_RES_X];
  static bool _gradation_dirty;
  static uint8_t _gradation_reds[HUB75_PANEL_RES_X];
  static uint8_t _gradation_greens[HUB75_PANEL_RES_X];
  static uint8_t _gradation_blues[HUB75_PANEL_RES_X];


public: 
  // h: 0 ~ 359
  static void h2rgb(uint16_t h, uint8_t& out_r, uint8_t& out_g, uint8_t& out_b);


protected:
  static uint32_t color_millis();


public:
  static void color_func_original(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_personal(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_rainbow_single(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_rainbow(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_gradation(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  
};



class FlexEffect : public Effect {
public:
  FlexEffect(const char* name) : Effect(name) {
  }
  
};



class FixedEffect : public Effect {
public:
  FixedEffect(const char* name) : Effect(name) {
  }
  
};

};
