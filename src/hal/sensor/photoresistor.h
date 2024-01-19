#pragma once


namespace toaster {

class Photoresistor {
public:
  Photoresistor() {}

public:
  bool begin(int pin, float alpha = 0.95f, float alpha_init = NAN);
  void loop();

  uint32_t getValue() const {
    return (uint32_t)_value;
  }

protected:
  int _pin{0};
  float _value{0};
  float _alpha{0.0f};
  float _alpha_init{0.0f};
};

};
