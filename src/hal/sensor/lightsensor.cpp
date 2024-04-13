#include <Arduino.h>
#include "lightsensor.h"
#include "lib/esp32_adc_lut.h"


namespace toaster {


static const uint32_t LIGHT_SENSOR_FREQUENCY = 30;


static float analogReadLUT(uint8_t pin) {
  uint16_t value = analogRead(pin);

  return ((value >= 4096) ? value : ADC_LUT[value]);
}


LightSensor::~LightSensor() {
  if (_type == LST_BH1750) {
    if (_bh1750 != nullptr) {
      delete _bh1750;
      _bh1750 = nullptr;
    }
  }
}


bool LightSensor::beginLDR(int pin, float alpha, float alpha_init) {
  _type = LST_LDR;

  _pin = pin;
  _alpha = alpha;
  _alpha_init = isnan(alpha_init) ? _alpha : _alpha_init;

  pinMode(_pin, INPUT);
  
  return _begin();
}


bool LightSensor::beginBH1750(uint8_t i2c, float alpha, float alpha_init) {
  _type = LST_LDR;
  
  _pin = i2c;
  _alpha = alpha;
  _alpha_init = isnan(alpha_init) ? _alpha : _alpha_init;

  _bh1750 = new BH1750;
  _bh1750->begin(BH1750::CONTINUOUS_LOW_RES_MODE, _pin);

  return _begin();
}


bool LightSensor::work() {
  if (_type == LST_NONE) {
    return false;
  }
  
  _value = (read() * (1 - _alpha)) + (_value * _alpha);
  return true;
}


bool LightSensor::_begin() {
  _value = read();
  for (int i = 1; i < 10; i++) {
    _value = (read() * (1 - _alpha_init)) + (_value * _alpha_init);
    delay(10);
  }

  Worker::begin(LIGHT_SENSOR_FREQUENCY);
  return true;
}


float LightSensor::read() {
  if (_type == LST_LDR) {
    return analogReadLUT(_pin);
  }
  if (_type == LST_BH1750) {
    return _bh1750->readLightLevel();
  }

  return 0.0f;
}


};
