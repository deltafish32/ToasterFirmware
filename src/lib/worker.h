#pragma once
#include <Arduino.h>
#include "timer.h"


namespace toaster {

class Worker {
public:
  Worker() {
  }

public:
  virtual void begin(uint32_t target_frequency) {
    _worker_tick_us = _fps_tick_us = Timer::get_micros();
    setFPS(target_frequency);
  }

  virtual void beginPeriod(uint32_t target_period_ms) {
    _worker_tick_us = _fps_tick_us = Timer::get_micros();
    setPeriod(target_period_ms);
  }

  virtual void setFPS(uint32_t target_frequency) {
    _worker_uspf = 1000000 / target_frequency;
    _worker_uspf_error = 1000000 % target_frequency;
  }

  virtual void setPeriod(uint32_t target_period_ms) {
    _worker_uspf = (timer_us_t)target_period_ms * 1000;
    _worker_uspf_error = 0;
  }

  virtual void setPF(timer_pf_t pf) {
    if (pf.type == PF_FREQUENCY) {
      setFPS(pf.frequency);
    }
    else if (pf.type == PF_PERIOD) {
      setPeriod(pf.period_ms);
    }
  }

  virtual void loop() {
    static const timer_us_t SECOND_US = 1000000;
    
    timer_us_t tick_us = Timer::get_micros();

    if (tick_us - _worker_tick_us >= _worker_uspf) {
      if (tick_us - _worker_tick_us >= _worker_uspf * 2) {
        _worker_tick_us = tick_us;
        // ++_frameSkips;
      }
      else {
        _worker_tick_us += _worker_uspf + ((_frames < _worker_uspf_error) ? 1 : 0);
      }

      if (work()) {
        ++_frames;
      }
    }
    
    if (tick_us - _fps_tick_us >= SECOND_US) {
      if (tick_us - _fps_tick_us >= SECOND_US * 2) {
        _fps_tick_us = tick_us;
      }
      else {
        _fps_tick_us += SECOND_US;
      }

      _recentFPS = _frames;
      
      workPerSecond();

      _frames = 0;
      // _frameSkips = 0;
    }

  }

protected:
  virtual bool work() = 0;
  virtual bool workPerSecond() { return true; };

private:
  timer_us_t _worker_tick_us{0};
  timer_us_t _worker_uspf{0};
  uint32_t _worker_uspf_error{0};

  uint32_t _frames{0};
  // uint32_t _frameSkips{0};
  timer_us_t _fps_tick_us{0};

  uint32_t _recentFPS{0};

public:
  uint32_t getRecentFPS() const {
    return _recentFPS;
  }

};

};
