#pragma once
#include "lib/worker.h"
#include "BH1750.h"


namespace toaster {

class LightSensor : public Worker {
public:
  LightSensor() {}
  virtual ~LightSensor();

private:
  virtual bool begin() { return false; }

public:
  virtual bool beginLDR(int pin, float alpha = 0.95f, float alpha_init = NAN);
  virtual bool beginBH1750(uint8_t i2c, float alpha = 0.95f, float alpha_init = NAN);
  virtual bool work();

  float getValue() const {
    return _value;
  }

protected:
  enum {
    LST_NONE = 0,
    LST_LDR,
    LST_BH1750,
  };

protected:
  uint8_t _type{LST_NONE};
  int _pin{0};
  float _value{0};
  float _alpha{0.0f};
  float _alpha_init{0.0f};

  union {
    void* _ptr{nullptr};
    BH1750* _bh1750;
  };

protected:
  bool _begin();
  float read();

};

};
