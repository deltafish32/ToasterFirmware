#include "neopixel_display.h"


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

  return true;
}


void NeoPixelDisplay::beginDraw() {
}


void NeoPixelDisplay::endDraw() {
  for (int i = 0; i < _width; i++) {
    int index = (i * 3);
    _neopixel->SetPixelColor(_reversed ? (_width - i - 1) : i, 
      RgbColor(calcBrightness(_buffer[index + 0], _brightness), 
               calcBrightness(_buffer[index + 1], _brightness), 
               calcBrightness(_buffer[index + 2], _brightness)));
  }

  _neopixel->Show();
  
  Display::endDraw();
}

};
