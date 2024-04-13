#pragma once
#include <vector>
#include "lib/worker.h"


namespace toaster {

class I2CESPNowRemote : public Worker {
public:
  I2CESPNowRemote();

public:
  virtual bool begin(uint8_t addr);

protected:
  virtual bool work();

protected:
  bool _init{false};
  uint8_t _i2c_addr{0};
  bool _i2c_found{false};
  std::vector<uint8_t> _i2c_buffer;

protected: 
  static uint8_t calcChecksum(const uint8_t* data, size_t len);
  
};

};
