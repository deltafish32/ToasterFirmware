#pragma once
#include <Arduino.h>


namespace toaster {

class Worker {
public:
  Worker() {
  }

public:
  virtual bool begin(uint32_t target_fps) {
    _worker_uspf = 1000000 / target_fps;
    _worker_tick_us = _fps_tick_us = micros();

    return true;
  }

  virtual void loop() {
    static const uint32_t SECOND_US = 1000000;
    
    uint32_t tick_us = micros();

    if (tick_us - _worker_tick_us >= _worker_uspf) {
      if (tick_us - _worker_tick_us >= _worker_uspf * 2) {
        _worker_tick_us = tick_us;
        // ++_frameSkips;
      }
      else {
        _worker_tick_us += _worker_uspf;
      }

      work();

      ++_frames;
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
  virtual void work() = 0;
  virtual void workPerSecond() {};

private:
  uint32_t _worker_uspf{0};
  uint32_t _worker_tick_us{0};

  uint32_t _frames{0};
  // uint32_t _frameSkips{0};
  uint32_t _fps_tick_us{0};

  uint32_t _recentFPS{0};

public:
  uint32_t getRecentFPS() const {
    return _recentFPS;
  }

};

};
