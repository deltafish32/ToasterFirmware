#pragma once
#include "config/configure.h"

#include "hal/display/display.h"
#include "lib/timer.h"
#include "lib/random.h"
#include "lib/linear_calibrate.h"

#include "protogen_color_mode.h"


namespace toaster {

class Effect : protected Random {
public:
  Effect(const char* name);
  virtual ~Effect();


public:
  virtual void init(Display& display) {
    setStep(0);
  }
  
  virtual void process(Display& display) = 0;
  virtual void release(Display& display) {
  }
  
  virtual const char* getName() const {
    return _name;
  }

  virtual bool isBlank() const {
    return false;
  }

  virtual bool isScript() const {
    return false;
  }

  virtual bool isNoShuffle() const {
    return false;
  }

  virtual bool isVideoMode() const {
    return false;
  }

  virtual bool isGame() const {
    return false;
  }

  virtual timer_pf_t getVideoPF() const {
    return {PF_NONE, };
  }

  virtual void restart() {
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

      timer_us_t tick_us = Timer::get_micros();
      
      if (_staticMode) {
        _static_mode_tick_us = tick_us;
      }
      else {
        timer_us_t tick_paused_us = tick_us - _static_mode_tick_us;
        for (auto& it : _effects) {
          it->_tick_us += tick_paused_us;
        }
        
        _color_tick_us += tick_us - _static_mode_tick_us;
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

  static void clearGradationPoint(uint8_t index) {
    _gradation_r[index].clear();
    _gradation_g[index].clear();
    _gradation_b[index].clear();

    setDirty();
  }

  static void setGradationPoint(uint8_t index, float p, uint8_t r, uint8_t g, uint8_t b) {
    _gradation_r[index].pushDataPoint({p, r});
    _gradation_g[index].pushDataPoint({p, g});
    _gradation_b[index].pushDataPoint({p, b});

    setDirty();
  }

  static void setDirty() {
    _rainbow_single_dirty = true;
    _rainbow_dirty = true;
    _gradation_dirty = true;

    _sync_timer_us = Timer::get_micros();
  }

  static Effect* find(const char* name, uint8_t script_index, const char* base_path);
  static Effect* findFromHardcoded(const char* name);
  static Effect* findFromScript(const char* name, uint8_t script_index, const char* base_path);


protected:
  static std::vector<Effect*> _effects;


protected:
  const char* _name{0,};
  int _step{0};
  timer_us_t _tick_us{0};
  static timer_us_t _sync_timer_us;

  void restartTimer() {
    _tick_us = _sync_timer_us;

    if (_staticMode) {
      _static_mode_tick_us = _tick_us;
    }
  }

  void setStep(int step) {
    _step = step;

    restartTimer();
  }

  bool timeout(uint32_t time_ms) {
    if (_staticMode) {
      return false;
    }

    return ((_sync_timer_us - _tick_us) >= (time_ms * 1000));
  }


public:
  enum {
    SM_NONE = 0,
    SM_DRAW,
    SM_DRAW_90,
    SM_DRAW_180,
    SM_DRAW_270,
    SM_VIDEO,
    SM_VIDEO_RESERVED_1,
    SM_VIDEO_RESERVED_2,
    SM_VIDEO_RESERVED_3,
    SM_VIDEO_LOOP,
    SM_VIDEO_LOOP_RESERVED_1,
    SM_VIDEO_LOOP_RESERVED_2,
    SM_VIDEO_LOOP_RESERVED_3,
  };

  enum {
    PC_ORIGINAL = 255,
    PC_EYES = 0,
    PC_NOSE,
    PC_MOUTH,
    PC_SIDE,

    PC_MAX
  };

  enum {
    DM_SINGLE = 0,
    DM_COPY,
    DM_MIRROR,
    DM_MIRROR_ONLY_OFFSET,
  };


protected:
  static bool _staticMode;
  static timer_us_t _static_mode_tick_us;
  static timer_us_t _color_tick_us;
  static PROTOGEN_COLOR_MODE _colorMode;
public:
  static COLOR_FUNC _colorFunc;
  static uint8_t _personalColor[8][3];
  static uint32_t _rainbowSingleSpeed_ms;
  static uint32_t _rainbowSpeed_ms;
  static uint32_t _rainbowTransitionPixels;
  static LinearCalibrate<float, uint32_t> _gradation_r[2];
  static LinearCalibrate<float, uint32_t> _gradation_g[2];
  static LinearCalibrate<float, uint32_t> _gradation_b[2];
  static bool _rainbow_single_dirty;
  static uint8_t _rainbow_single_red;
  static uint8_t _rainbow_single_green;
  static uint8_t _rainbow_single_blue;
  static bool _rainbow_dirty;
  static uint8_t _rainbow_red[HUB75_PANEL_RES_X];
  static uint8_t _rainbow_green[HUB75_PANEL_RES_X];
  static uint8_t _rainbow_blue[HUB75_PANEL_RES_X];
  static bool _gradation_dirty;
  static uint8_t _gradation_reds[2][HUB75_PANEL_RES_X];
  static uint8_t _gradation_greens[2][HUB75_PANEL_RES_X];
  static uint8_t _gradation_blues[2][HUB75_PANEL_RES_X];


public: 
  // h: 0 ~ 359
  static void h2rgb(uint16_t h, uint8_t& out_r, uint8_t& out_g, uint8_t& out_b);


protected:
  static timer_ms_t color_millis();


public:
  static void color_func_original(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_personal(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_rainbow_single(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_rainbow(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_gradation(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  
  static void color_func_gray_personal(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_gray_rainbow_single(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_gray_rainbow(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  static void color_func_gray_gradation(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t a, uint8_t param);
  
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
