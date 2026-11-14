#include "neopixel_display.h"
#include <new>


namespace toaster {

NeoPixelDisplay::NeoPixelDisplay() {
}


bool NeoPixelDisplay::begin(int numleds, int pin, bool reversed) {
  if (!Display::begin(numleds, 1)) {
    return false;
  }
  
  _neopixel = new NEOPIXEL_TYPE(numleds, pin);
  _neopixel->Begin();
  _neopixel->Show();

  _reversed = reversed;

  if (_prev_buffer != nullptr) {
    delete[] _prev_buffer;
  }
  _prev_buffer = new (std::nothrow) uint8_t[_width * _height * 3];

  return (_prev_buffer != nullptr);
}


void NeoPixelDisplay::off() {
  if (_neopixel == nullptr) {
    return;
  }

  _neopixel->ClearTo(RgbColor(0));
  _neopixel->Show();
}


void NeoPixelDisplay::beginDraw() {
}


void NeoPixelDisplay::endDraw() {
  if (memcmp(_prev_buffer, _buffer, _width * _height * 3) != 0) {
    memcpy(_prev_buffer, _buffer, _width * _height * 3);

    for (int i = 0; i < _width; i++) {
      int index = (i * 3);
      _neopixel->SetPixelColor(_reversed ? (_width - i - 1) : i, 
        RgbColor(calcBrightness(_buffer[index + 0], _brightness), 
                 calcBrightness(_buffer[index + 1], _brightness), 
                 calcBrightness(_buffer[index + 2], _brightness)));
    }
  }

  _neopixel->Show();
  
  Display::endDraw();
}

};
