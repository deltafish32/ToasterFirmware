#include <Arduino.h>
#include <Wire.h>
#include "boopsensor.h"

#include "lib/logger.h"


namespace toaster {

static const char* TAG = "BoopSensor";

static const uint32_t TOF_FREQUENCY = 50;
static const uint32_t TOF_TIMEOUT_MS = 100;
static const uint32_t TOF_INIT_TIMEOUT_MS = 500;
static const uint32_t BOOP_TIMEOUT_MS = 1000;


bool BoopSensor::begin(uint8_t i2c_addr, uint16_t threshold, int sampling) {
  if (_init) {
    return true;
  }

  _i2c = i2c_addr;
  _threshold = threshold;
  _sampling = sampling;

  if (!tof_init()) {
    return false;
  }
  
  Worker::begin(TOF_FREQUENCY);

  _init = true;

  return true;
}


bool BoopSensor::work() {
  if (_init == false || _error) {
    return false;
  }

  timer_ms_t tick_ms = Timer::get_millis();

  uint8_t dataReady = false;
  if (VL53L1_ERROR_NONE != _tof.CheckForDataReady(&dataReady)) {
    tof_error();
    return false;
  }

  if (!dataReady) {
    if (Timer::get_millis() - tick_ms >= TOF_TIMEOUT_MS) {
      tof_error(true);
    }

    return false;
  }

  if (VL53L1_ERROR_NONE != _tof.GetDistanceInMm(&_value)) {
    tof_error();
    return false;
  }

  if (_debug) {
    TF_LOGI(TAG, "%d", _value);
  }

  _boop_tick_ms = Timer::get_millis();
  bool boop = (_value >= 0 && _value <= _threshold);

  if (_boop != boop) {
    ++_boop_count;
    if (_boop_count >= _sampling) {
      _boop = !_boop;
      _boop_count = 0;
    }
  }
  else {
    _boop_count = 0;
  }

  if (VL53L1_ERROR_NONE != _tof.ClearInterrupt()) {
    tof_error();
    return false;
  }

  return true;
}


bool BoopSensor::isBoop() const {
  if (_enabled == false) {
    return false;
  }

  if (_emulation) {
    return true;
  }
  
  if (_init == false || _error) {
    return false;
  }

  if (Timer::get_millis() - _boop_tick_ms >= BOOP_TIMEOUT_MS) {
    return false;
  }

  return _boop;
}


bool BoopSensor::tof_init() {
  timer_ms_t tick_ms = Timer::get_millis();
  while (1) {
    Wire.beginTransmission(_i2c);
    uint8_t result = Wire.endTransmission();
    if (result == 0) {
      break;
    }
    if (Timer::get_millis() - tick_ms >= TOF_TIMEOUT_MS) {
      return false;
    }
    delay(20);
  }

  VL53L1_Error status = _tof.Begin(_i2c << 1); // VL53L1X_ULD shifts the address internally
  if (status != VL53L1_ERROR_NONE) {
    TF_LOGW(TAG, "VL53L1X Begin failed %d", status);
    return false;
  }

  // Short(1.3m) or Long(4.0m)
  // status = _tof.SetDistanceMode(Short);
  status = _tof.SetDistanceMode(Long);
  if (status != VL53L1_ERROR_NONE) {
    TF_LOGE(TAG, "VL53L1X SetDistanceMode failed %d", status);
    return false;
  }
  
  // Allowed values: 15, 20, 33, 50, 100, 200, 500
  // uint16_t timing_budget = 15;
  uint16_t timing_budget = 20;
  status = _tof.SetTimingBudgetInMs(timing_budget);
  if (status != VL53L1_ERROR_NONE) {
    TF_LOGE(TAG, "VL53L1X SetTimingBudgetInMs failed %d", status);
    return false;
  }

  status = _tof.SetInterMeasurementInMs(timing_budget + 5);
  if (status != VL53L1_ERROR_NONE) {
    TF_LOGE(TAG, "VL53L1X SetInterMeasurementInMs failed %d", status);
    return false;
  }

  return (VL53L1_ERROR_NONE == _tof.StartRanging());
}

void BoopSensor::tof_error(bool critical) {
  ++_error_total;
  ++_errors;

  if (critical || _errors >= 2) {
    _errors = 0;

    if (critical || !tof_init()) {
      _error = true;

      TF_LOGE(TAG, "VL53L1X reset failed. now disabled.");
    }
    else {
      TF_LOGD(TAG, "VL53L1X reset done.");
    }
  }
}

void BoopSensor::tof_no_error() {
  _errors = 0;
}

};
