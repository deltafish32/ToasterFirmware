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
  uint8_t spare{0};
  uint16_t battery{0};
} ESP_NOW_DATA;



I2CESPNowRemote::I2CESPNowRemote() {
}


bool I2CESPNowRemote::begin(uint8_t addr) {
  if (_init) {
    return true;
  }

  _i2c_addr = addr;

  // delay(2000);
  // for (int i = 0; i < 127; i++) {
  //   Wire.beginTransmission(i);
  //   if (Wire.endTransmission() == 0) {
  //     TF_LOGD(TAG, "Found: 0x%02x", i);
  //   }
  // }

  Wire.beginTransmission(_i2c_addr);
  uint8_t result = Wire.endTransmission();
  if (result == 0) {
    _i2c_found = true;
  }

  Worker::begin(60);

  _init = true;

  return _i2c_found;
}


bool I2CESPNowRemote::work() {
  if (_init == false || _i2c_found == false) {
    return false;
  }

	_i2c_buffer.clear();
  
	Wire.requestFrom(_i2c_addr, (uint8_t)11);
	
	while (Wire.available()) {
		while (_i2c_buffer.size() >= I2C_BUFFER_MAX) {
			_i2c_buffer.erase(_i2c_buffer.begin());
		}
		
		_i2c_buffer.push_back(Wire.read());
	}
	
	while (_i2c_buffer.size() >= 11) {
		uint8_t checksum = calcChecksum(&_i2c_buffer[0], 10);
		if (checksum == _i2c_buffer[10]) {
			ESP_NOW_DATA data;
			
			for (int i = 0; i < 6; i++) {
				data.mac[i] = _i2c_buffer[i];
			}
			data.data = _i2c_buffer[6];
      data.spare = _i2c_buffer[7];
      data.battery = (_i2c_buffer[8] | (_i2c_buffer[9] << 8));
			
      if (data.mac[0] != 0 && data.mac[1] != 0 && data.mac[2] != 0 && data.mac[3] != 0 && data.mac[4] != 0 && data.mac[5] != 0 && data.data != 0) {
        Protogen._hud.pressKey(data.data);
      }
		}

    for (int i = 0; i < 11; i++) {
      _i2c_buffer.erase(_i2c_buffer.begin());
    }
	}

  return true;
}


uint8_t I2CESPNowRemote::calcChecksum(const uint8_t* data, size_t len) {
  uint32_t checksum = 0;
  
  for (size_t i = 0; i < len; i++) {
    checksum += data[i];
  }
  
  return (uint8_t)checksum;
}

};
