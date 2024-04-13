#pragma once
#include "VL53L1X_ULD.h"
#include "lib/worker.h"


namespace toaster {

class BoopSensor : public Worker {
public:
  BoopSensor() {}

public:
  virtual bool begin(uint8_t i2c_addr, uint16_t threshold, int sampling);
  virtual bool work();

  bool isBoop() const;

  uint16_t getDistance_mm() const {
    return _value;
  }

  bool isError() const {
    return (!_init || _error);
  }

  uint32_t getErrorTotal() const {
    return _error_total;
  }

  void setDebug(bool debug) {
    _debug = debug;
  }

  bool getDebug() const {
    return _debug;
  }

  void setEnabled(bool enabled) {
    _enabled = enabled;
  }

  bool getEnabled() const {
    return _enabled;
  }
  
  void setEmulation(bool emulation) {
    _emulation = emulation;
  }

  bool getEmulation() const {
    return _emulation;
  }

protected:
  bool tof_init();
  void tof_error(bool critical = false);
  void tof_no_error();

protected:
  bool _init{false};
  VL53L1X_ULD _tof;
  uint8_t _i2c{0x29};
  uint16_t _threshold{20};
  int _sampling{3};
  bool _debug{false};
  bool _enabled{true};
  bool _emulation{false};

protected:
  uint16_t _value{0};
  int _boop_count{0};
  bool _boop{false};
  timer_ms_t _boop_tick_ms{0};

protected:
  bool _error{false};
  uint32_t _errors{0};
  uint32_t _error_total{0};

};

};
