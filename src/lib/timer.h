#pragma once
#include <Arduino.h>


namespace toaster {

typedef int64_t timer_us_t;
typedef int64_t timer_ms_t;


enum {
  PF_NONE = 0,
  PF_FREQUENCY,
  PF_PERIOD,
};


typedef struct _timer_pf_t {
  uint8_t type;

  union {
    uint32_t frequency;
    uint32_t period_ms;
  };
} timer_pf_t;


class Timer {
private:
  Timer() {}

public:
  static timer_us_t get_micros() {
    return esp_timer_get_time();
  }
  
  static timer_ms_t get_millis() {
    return esp_timer_get_time() / 1000;
  }

};

};
