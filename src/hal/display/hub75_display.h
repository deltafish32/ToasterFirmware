#pragma once

#include "display.h"

#include <FFat.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>



namespace toaster {

class Hub75Display : public Display {
public:
  Hub75Display();


public:
  virtual bool begin(int width, int height, int chain);
  virtual bool begin(int width, int height, int chain, HUB75_I2S_CFG::i2s_pins pins);


public:
  virtual void beginDraw();
  virtual void endDraw();


public:
  virtual bool draw_png_newcolor(upng_t* upng, COLOR_FUNC color_func, uint8_t param = 0, DRAW_MODE draw_mode = DRAW_DEFAULT, int offset_x = 0, int offset_y = 0, int rotate_cw = 0);

  // virtual bool undraw_png(upng_t* upng, DRAW_MODE draw_mode = DRAW_DEFAULT, int offset_x = 0, int offset_y = 0, int rotate_cw = 0);


protected:
  MatrixPanel_I2S_DMA *_dma_display{nullptr};
  uint8_t _chain{0};
  size_t _panel_width{0};

};

};
