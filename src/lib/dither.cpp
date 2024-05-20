#include "config/configure.h"
#include "dither.h"


namespace toaster {


// Source referenced: https://github.com/deeptronix/dithering_halftoning


static inline void quantize_BW(uint8_t &c) {
  c = (int8_t)c >> 7;
}


static inline uint8_t clamp(int16_t v, uint8_t min, uint8_t max) {
  uint16_t t = (v) < min ? min : (v);
  return t > max ? max : t;
}


// Fast Error Diffusion Dithering algorithm
void fastEDDither(uint8_t *IMG_pixel) {
  static const uint16_t IMG_WIDTH = HUB75_PANEL_RES_X * HUB75_PANEL_CHAIN;
  static const uint16_t IMG_HEIGHT = HUB75_PANEL_RES_Y;
  
  for (uint16_t row = 0; row < IMG_HEIGHT - 1; row++) {
    for (uint16_t col = 0; col < IMG_WIDTH - 1; col++) {
      uint32_t ind = (row * IMG_WIDTH) + col;
      uint8_t c = IMG_pixel[ind];
      uint8_t newc = c;
      quantize_BW(newc);
      int8_t quant_err_c = (c - newc) >> 1;
      IMG_pixel[ind] = newc;
      
      // adjacent pixels work starts here:
      // distribute part of error at (x + 1, y)      
      c = IMG_pixel[ind + 1];
      uint16_t temp_c = c + quant_err_c;
      IMG_pixel[ind + 1] = clamp(temp_c, 0, 255);
      
      // distribute part of error at (x, y + 1)
      c = IMG_pixel[ind + IMG_WIDTH];
      temp_c = c + quant_err_c; // distribute the whole quantization error to the pixel below  
      IMG_pixel[ind + IMG_WIDTH] = clamp(temp_c, 0, 255);
    }
    
    uint32_t ind = (row * IMG_WIDTH) + (IMG_WIDTH - 1);
    uint8_t c = IMG_pixel[ind];
    uint8_t newc = c;
    quantize_BW(newc);
    int8_t quant_err_c = (c - newc) >> 1;
    IMG_pixel[ind] = newc;
    
    // distribute part of error at (x, y + 1)
    c = IMG_pixel[ind + IMG_WIDTH];
    uint8_t temp_c = c + quant_err_c; // distribute the whole quantization error to the pixel below
    IMG_pixel[ind + IMG_WIDTH] = clamp(temp_c, 0, 255);
  }
  
  uint32_t ind = ((IMG_HEIGHT - 1) * IMG_WIDTH) + (IMG_WIDTH - 1);
  uint8_t c = IMG_pixel[ind];
  uint8_t newc = c;
  quantize_BW(newc);
  int8_t quant_err_c = (c - newc) >> 1;
  IMG_pixel[ind] = newc;
}


};
