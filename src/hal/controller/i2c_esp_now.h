#pragma once
#include <vector>
#include "lib/worker.h"


namespace toaster {

class I2CESPNowRemote : public Worker {
public:
  I2CESPNowRemote();

public:
  virtual bool begin(uint8_t addr);

public:
  void clearWhitelist();
  void addWhitelist(const uint8_t* mac);
  bool checkWhitelist(const uint8_t* mac) const;

protected:
  virtual bool work();

protected:
  bool _init{false};
  uint8_t _i2c_addr{0};
  bool _i2c_found{false};

protected:
  typedef struct _MAC_ADDRESS {
    uint8_t mac[6];
  } MAC_ADDRESS;
  std::vector<MAC_ADDRESS> _whitelist;

protected: 
  static uint8_t calcChecksum(const uint8_t* data, size_t len);
  
protected:
  uint8_t commReadLength();
  void commReadData(uint8_t len);
  
};

};
