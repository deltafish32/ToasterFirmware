#include <Arduino.h>
#include "thermometer.h"
#include "lib/logger.h"
#include "protogen.h"


namespace toaster {


Thermometer::~Thermometer() {
  if (_type == TST_SHT31) {
    if (_sht31 != nullptr) {
      delete _sht31;
      _sht31 = nullptr;
    }
  }
}


bool Thermometer::beginSHT31(uint32_t period, uint8_t i2c) {
  _type = TST_SHT31;

  _pin = i2c;
  _sht31 = new Adafruit_SHT31;

  if (!_sht31->begin(_pin)) {
    delete _sht31;
    _sht31 = nullptr;
    return false;
  }

  beginPeriod(period);
  work();

  return true;
}



bool Thermometer::work() {
  if (!isBegin()) {
    return false;
  }
  
  if (_type == TST_SHT31) {
    float temperature;
    float humidity;
    if (!_sht31->readBoth(&temperature, &humidity)) {
      return false;
    }

    if (!isnan(temperature)) {
      _temperature = temperature;
    }
    
    if (!isnan(humidity)) {
      _humidity = humidity;
    }

    _read_tick_ms = Timer::get_millis();

    if (!Protogen._serialdebug.isTextMode()) {
      TF_LOGD("Thermometer", "Temperature: %.2f\"C, Humidity: %.2f%%", _temperature, _humidity);
    }

    return true;
  }
  
  return false;
}


};
