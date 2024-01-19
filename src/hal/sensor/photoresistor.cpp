#include <Arduino.h>
#include "photoresistor.h"
#include "lib/esp32_adc_lut.h"


namespace toaster {

static float analogReadLUT(uint8_t pin) {
  uint16_t value = analogRead(pin);

  return ((value >= 4096) ? value : ADC_LUT[value]);
}


bool Photoresistor::begin(int pin, float alpha, float alpha_init) {
  _pin = pin;
  _alpha = alpha;
  _alpha_init = isnan(alpha_init) ? _alpha : _alpha_init;

  pinMode(_pin, INPUT);

  _value = analogReadLUT(_pin);
  for (int i = 1; i < 10; i++) {
    _value = (analogReadLUT(_pin) * (1 - _alpha_init)) + (_value * _alpha_init);
    delay(10);
  }

  return true;
}


void Photoresistor::loop() {
  _value = (analogReadLUT(_pin) * (1 - _alpha)) + (_value * _alpha);
}

};
