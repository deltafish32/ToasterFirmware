#include <Arduino.h>
#include "i2c_esp_now.h"

#include <Wire.h>
#include "huds/hud.h"
#include "protogen.h"
#include "lib/logger.h"


namespace toaster {

static const char* TAG = "I2CESPNowRemote";
static const size_t I2C_BUFFER_MAX = 16;


typedef struct _ESP_NOW_DATA {
	uint8_t mac[6]{0,};
	uint8_t data{0};
} ESP_NOW_DATA;



I2CESPNowRemote::I2CESPNowRemote() {
}


bool I2CESPNowRemote::begin(uint8_t addr) {
  if (_init) {
    return true;
  }

  _i2c_addr = addr;

	Wire.begin();

  // delay(2000);
  // for (int i = 0; i < 127; i++) {
  //   Wire.beginTransmission(i);
  //   if (Wire.endTransmission() == 0) {
  //     TF_LOG(TAG, "Found: 0x%02x\n", i);
  //   }
  // }

  Wire.beginTransmission(_i2c_addr);
  uint8_t result = Wire.endTransmission();
  if (result == 0) {
    _i2c_found = true;
  }

  if (_i2c_found) {
    TF_LOG(TAG, "ESP-Now I2C 0x%02x connected\n", _i2c_addr);
  }
  else {
    TF_LOG(TAG, "ESP-Now I2C 0x%02x not connected (%d)\n", _i2c_addr, result);
  }

  if (!Worker::begin(60)) {
    return false;
  }

  _init = true;

  return _i2c_found;
}


void I2CESPNowRemote::work() {
  if (_i2c_found == false) return;

	_i2c_buffer.clear();
  
	Wire.requestFrom(_i2c_addr, (uint8_t)8);
	
	while (Wire.available()) {
		while (_i2c_buffer.size() >= I2C_BUFFER_MAX) {
			_i2c_buffer.erase(_i2c_buffer.begin());
		}
		
		_i2c_buffer.push_back(Wire.read());
	}
	
	while (_i2c_buffer.size() >= 8) {
		uint8_t checksum = calcChecksum(&_i2c_buffer[0], 7);
		if (checksum == _i2c_buffer[7]) {
			ESP_NOW_DATA data;
			
			for (int i = 0; i < 6; i++) {
				data.mac[i] = _i2c_buffer[i];
			}
			data.data = _i2c_buffer[6];
			
      switch (data.data) {
      case 'S': case 's':
        Protogen._hud.pressKey('S');
        break;
      case 'D': case 'd':
        Protogen._hud.pressKey('D');
        break;
      case 'F': case 'f':
        Protogen._hud.pressKey('F');
        break;
      }
		}

    for (int i = 0; i < 8; i++) {
      _i2c_buffer.erase(_i2c_buffer.begin());
    }
	}
}


uint8_t I2CESPNowRemote::calcChecksum(const uint8_t* data, size_t len) {
  uint32_t checksum = 0;
  
  for (size_t i = 0; i < len; i++) {
    checksum += data[i];
  }
  
  return (uint8_t)checksum;
}

};
