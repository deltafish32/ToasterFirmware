#include "neopixel_display.h"


namespace toaster {

NeoPixelDisplay::NeoPixelDisplay() {
}


bool NeoPixelDisplay::begin(int numleds, int pin) {
  if (!Display::begin(numleds, 1)) {
    return false;
  }
  
  _neopixel = new NEOPIXEL_TYPE(numleds, pin);
  _neopixel->Begin();
  _neopixel->Show();

  return true;
}


void NeoPixelDisplay::beginDraw() {
}


void NeoPixelDisplay::endDraw() {
  for (int i = 0; i < _width; i++) {
    int index = (i * 3);
    _neopixel->SetPixelColor(i, RgbColor(calcBrightness(_buffer[index + 0], _brightness), 
      calcBrightness(_buffer[index + 1], _brightness), 
      calcBrightness(_buffer[index + 2], _brightness)));
  }

  _neopixel->Show();
  
  Display::endDraw();
}

};
