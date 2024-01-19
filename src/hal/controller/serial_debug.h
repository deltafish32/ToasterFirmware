#pragma once
#include <vector>


namespace toaster {

class SerialDebug {
public:
  SerialDebug();

public:
  bool begin();
  void loop();

protected:
  std::vector<char> _serial_buffer;
  bool serial_text_mode{false};

protected:
  const char* serial_buffer_read();

};

};
