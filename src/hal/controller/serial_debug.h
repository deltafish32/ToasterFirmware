#pragma once
#include <vector>
#include "huds/hud_base.h"


namespace toaster {

class SerialDebug {
public:
  SerialDebug();

public:
  bool begin();
  void loop();

public:
  bool isTextMode() const {
    return _serial_text_mode;
  }

  bool pressKey(uint16_t key, uint8_t mode = KM_ASCII);
  bool processCommand(const char* cmd);

protected:
  std::vector<char> _serial_buffer;
  bool _serial_text_mode{false};
  int _cursor{0};

protected:
  const char* serial_buffer_read();
  void printAfterCursor(bool erase = false);

};

};
