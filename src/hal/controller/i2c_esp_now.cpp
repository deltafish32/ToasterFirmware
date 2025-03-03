#include <Arduino.h>
#include "i2c_esp_now.h"

#include <Wire.h>
#include "huds/hud.h"
#include "protogen.h"
#include "lib/logger.h"
#include "lib/esp32_adc_lut.h"
#include "esp_now_data.h"


namespace toaster {

static const char* TAG = "I2CESPNowRemote";



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
    
    uint8_t write_buffer[3] = {'R', 'M', };
    write_buffer[2] = calcChecksum(write_buffer, 2);
    Wire.beginTransmission(_i2c_addr);
    Wire.write(write_buffer, 3);
    Wire.endTransmission();

    const size_t BUFFER_SIZE = 7;
    uint8_t buffer[BUFFER_SIZE];

    Wire.requestFrom(_i2c_addr, (uint8_t)BUFFER_SIZE);
    size_t buffer_read = Wire.readBytes(buffer, BUFFER_SIZE);

    if (buffer_read == BUFFER_SIZE) {
      uint8_t checksum = calcChecksum(buffer, BUFFER_SIZE - 1);
      if (checksum == buffer[BUFFER_SIZE - 1]) {
        TF_LOGI(TAG, "ESP-Now Receiver MAC: %02x:%02x:%02x:%02x:%02x:%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
      }
    }
    else {
      // TF_LOGW(TAG, "checksum error (%02d but %02d)", checksum, buffer[BUFFER_SIZE - 1]);
    }
	}
  else {
    // TF_LOGW(TAG, "length error");
  }

  Worker::begin(60);

  _init = true;

  return _i2c_found;
}


void I2CESPNowRemote::clearWhitelist() {
  _whitelist.clear();
}


void I2CESPNowRemote::addWhitelist(const uint8_t* mac) {
  MAC_ADDRESS mac_address;
  memcpy(&mac_address, mac, sizeof(MAC_ADDRESS));
  _whitelist.push_back(mac_address);
}


bool I2CESPNowRemote::checkWhitelist(const uint8_t* mac) const {
  if (_whitelist.empty()) {
    return true;
  }

  for (const auto& it : _whitelist) {
    if (memcmp(&it, mac, sizeof(MAC_ADDRESS)) == 0) {
      return true;
    }
  }

  return false;
}


bool I2CESPNowRemote::work() {
  if (_init == false || _i2c_found == false) {
    return false;
  }

  uint8_t len = commReadLength();
  if (len > 0) {
    commReadData(len);
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


uint8_t I2CESPNowRemote::commReadLength() {
  uint8_t write_buffer[3] = {'R', 'L', };
  write_buffer[2] = calcChecksum(write_buffer, 2);

  Wire.beginTransmission(_i2c_addr);
  Wire.write(write_buffer, 3);
  Wire.endTransmission();
  
  static const uint8_t BUFFER_SIZE = 2;
  uint8_t buffer[BUFFER_SIZE];

	Wire.requestFrom(_i2c_addr, (uint8_t)BUFFER_SIZE);
  size_t buffer_read = Wire.readBytes(buffer, BUFFER_SIZE);
	
  if (buffer_read != BUFFER_SIZE) {
    TF_LOGW(TAG, "commReadLength: length error (%d but %d)", BUFFER_SIZE, buffer_read);
    return 0;
  }

  uint8_t checksum = calcChecksum(buffer, BUFFER_SIZE - 1);
  if (checksum != buffer[BUFFER_SIZE - 1]) {
    TF_LOGW(TAG, "commReadLength: checksum error (%02d but %02d)", checksum, buffer[BUFFER_SIZE - 1]);
    return 0;
  }

  return buffer[0];
}


void I2CESPNowRemote::commReadData(uint8_t len) {
  if (len < sizeof(ESP_NOW_DATA_BASE)) {
    return;
  }

  uint8_t write_buffer[3] = {'R', 'D', };
  write_buffer[2] = calcChecksum(write_buffer, 2);
  Wire.beginTransmission(_i2c_addr);
  Wire.write(write_buffer, 3);
  Wire.endTransmission();
  
  const size_t BUFFER_SIZE = len + 1;
  uint8_t buffer[BUFFER_SIZE];

	Wire.requestFrom(_i2c_addr, (uint8_t)BUFFER_SIZE);
  size_t buffer_read = Wire.readBytes(buffer, BUFFER_SIZE);
	
  if (buffer_read != BUFFER_SIZE) {
    TF_LOGW(TAG, "commReadData: length error (%d but %d)", BUFFER_SIZE, buffer_read);
    return;
  }

  uint8_t checksum = calcChecksum(buffer, BUFFER_SIZE - 1);
  if (checksum != buffer[BUFFER_SIZE - 1]) {
    TF_LOGW(TAG, "commReadData: checksum error (%02d but %02d)", checksum, buffer[BUFFER_SIZE - 1]);
    return;
  }

  if (buffer[6] == 'T' && buffer[7] == 'F') {
    if (len < sizeof(ESP_NOW_DATA_REMOTE)) {
      TF_LOGW(TAG, "commReadData: TF length error");
      return;
    }

    ESP_NOW_DATA_REMOTE data;
    memcpy(&data, buffer, BUFFER_SIZE - 1);
    
    // TF_LOGD(TAG, "recv mac: %02x:%02x:%02x:%02x:%02x:%02x", data.mac[0], data.mac[1], data.mac[2], data.mac[3], data.mac[4], data.mac[5]);
    // uint16_t battery = (data.battery < 4096) ? ADC_LUT[data.battery] : data.battery;
    // TF_LOGD(TAG, "%d", battery);

    if (checkWhitelist(data.mac)) {
      if (data.sign[0] == 'T' && data.sign[1] == 'F') {
        if (data.data != 0) {
          Protogen._hud.pressKey(data.data);
        }
      }
    }
    else {
      TF_LOGW(TAG, "unregistered mac: %02x:%02x:%02x:%02x:%02x:%02x", data.mac[0], data.mac[1], data.mac[2], data.mac[3], data.mac[4], data.mac[5]);
    }
  }
}


};
