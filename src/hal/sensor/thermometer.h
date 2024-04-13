#pragma once
#include "lib/worker.h"
#include "Adafruit_SHT31.h"


namespace toaster {

class Thermometer : public Worker {
public:
  Thermometer() {}
  virtual ~Thermometer();

private:
  bool begin() { return false; }

public:
  bool beginSHT31(uint32_t period, uint8_t i2c = 0x44);
  virtual bool work();

  float getTemperature() const {
    return _temperature;
  }

  float getHumidity() const {
    return _humidity;
  }

  bool isBegin() const {
    return (_ptr != nullptr);
  }

protected:
  enum {
    TST_NONE = 0,
    TST_SHT31,
  };

protected:
  uint8_t _type{TST_NONE};
  int _pin{0};
  float _temperature{NAN};
  float _humidity{NAN};
  timer_ms_t _read_tick_ms{0};

  union {
    void* _ptr{nullptr};
    Adafruit_SHT31* _sht31;
  };

};

};
