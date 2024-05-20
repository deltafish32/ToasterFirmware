#include "config/configure.h"
#include "hub75_display.h"
#include "lib/logger.h"

#include "config/configure_mask.h"


namespace toaster {

static const char* TAG = "Hub75Display";


Hub75Display::Hub75Display() {
}


bool Hub75Display::begin(int width, int height, int chain) {
  return begin(width, height, chain, {R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT,
    A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT,
    LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT}, 240);
}

bool Hub75Display::begin(int width, int height, int chain, HUB75_I2S_CFG::i2s_pins pins, uint8_t min_refresh_rate) {
  if (!Display::begin(width * chain, height)) {
    return false;
  }

  _chain = chain;
  _panel_width = width;

  HUB75_I2S_CFG mxconfig(_panel_width, _height, _chain);

  mxconfig.gpio = pins;

  mxconfig.clkphase = false;
  mxconfig.latch_blanking = 4;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.min_refresh_rate = min_refresh_rate;

  _dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  if (!_dma_display->begin()) {
    return false;
  }

  _dma_display->setBrightness8(0);
  _dma_display->clearScreen();

  return true;
}


void Hub75Display::beginDraw() {
  _dma_display->setBrightness8(_brightness);
}


void Hub75Display::endDraw() {
  for (int y = 0; y < _height; y++) {
    for (int x = 0; x < _width; x++) {
      int px = ((x < _panel_width) ? (x) : (_width - x - 1));
      int pindex = (y * _panel_width + px);
      if ((CLIP_MASK[(pindex / 8)] & (0x80 >> (pindex % 8))) == 0) {
        _dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
      else {
        int index = (y * _width + x) * 3;
        _dma_display->drawPixelRGB888(x, y, _buffer[index + 0], _buffer[index + 1], _buffer[index + 2]);
      }
    }
  }

  Display::endDraw();
}


bool Hub75Display::draw_image_newcolor(const Image* image, COLOR_FUNC color_func, uint8_t param, DRAW_MODE draw_mode, int offset_x, int offset_y, int rotate_cw) {
  if (image == nullptr) {
    TF_LOGE(TAG, "image draw failed (nullptr).");
    return false;
  }

  auto width = image->getWidth();
  auto height = image->getHeight();
  auto bpp = image->getBpp();
  auto has_alpha = image->getHasAlpha();
  auto buffer = image->getBuffer();

  if (draw_mode == DRAW_SINGLE) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int index = (y * width + x) * (bpp + has_alpha);
        uint8_t a = has_alpha ? buffer[index + bpp] : 255;
        if (a == 0) continue;
        
        uint8_t r1;
        uint8_t g1;
        uint8_t b1;

        if (bpp == 3) {
          r1 = buffer[index + 0];
          g1 = buffer[index + 1];
          b1 = buffer[index + 2];
        }
        else {
          Image::rgb565be_to_rgb888(*((uint16_t*)(buffer + index)), r1, g1, b1);
        }
        
        int xr, yr;
        xy_rotate(xr, yr, x, y, width, height, rotate_cw);
        
        int x1 = xr + offset_x;
        int y1 = yr + offset_y;

        color_func(x1, y1, r1, g1, b1, a, param);

        int index0 = (y1 * _width + x1) * 3;
        uint8_t r0 = _buffer[index0 + 0];
        uint8_t g0 = _buffer[index0 + 1];
        uint8_t b0 = _buffer[index0 + 2];

        uint8_t r2 = (uint8_t)(((uint16_t)r1 * a / 255) + ((uint16_t)r0 * (255 - a) / 255));
        uint8_t g2 = (uint8_t)(((uint16_t)g1 * a / 255) + ((uint16_t)g0 * (255 - a) / 255));
        uint8_t b2 = (uint8_t)(((uint16_t)b1 * a / 255) + ((uint16_t)b0 * (255 - a) / 255));
        setPixel(x1, y1, r2, g2, b2);
      }
    }
  }
  else if (draw_mode == DRAW_COPY) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int index = (y * width + x) * (bpp + has_alpha);
        uint8_t a = has_alpha ? buffer[index + bpp] : 255;
        if (a == 0) continue;
        
        uint8_t r1;
        uint8_t g1;
        uint8_t b1;

        if (bpp == 3) {
          r1 = buffer[index + 0];
          g1 = buffer[index + 1];
          b1 = buffer[index + 2];
        }
        else {
          Image::rgb565be_to_rgb888(*((uint16_t*)(buffer + index)), r1, g1, b1);
        }
        
        int xr, yr;
        xy_rotate(xr, yr, x, y, width, height, rotate_cw);
        
        int x1 = xr + offset_x;
        int y1 = yr + offset_y;

        color_func(x1, y1, r1, g1, b1, a, param);

        int index0 = (y1 * _width + x1) * 3;
        uint8_t r0 = _buffer[index0 + 0];
        uint8_t g0 = _buffer[index0 + 1];
        uint8_t b0 = _buffer[index0 + 2];

        uint8_t r2 = (uint8_t)(((uint16_t)r1 * a / 255) + ((uint16_t)r0 * (255 - a) / 255));
        uint8_t g2 = (uint8_t)(((uint16_t)g1 * a / 255) + ((uint16_t)g0 * (255 - a) / 255));
        uint8_t b2 = (uint8_t)(((uint16_t)b1 * a / 255) + ((uint16_t)b0 * (255 - a) / 255));
        setPixel(x1, y1, r2, g2, b2);

        int x2 = x1 + _panel_width;
        int y2 = y1;
        setPixel(x2, y2, r2, g2, b2);
      }
    }
  }
  else if (draw_mode == DRAW_MIRROR) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int index = (y * width + x) * (bpp + has_alpha);
        uint8_t a = has_alpha ? buffer[index + bpp] : 255;
        if (a == 0) continue;
        
        uint8_t r1;
        uint8_t g1;
        uint8_t b1;

        if (bpp == 3) {
          r1 = buffer[index + 0];
          g1 = buffer[index + 1];
          b1 = buffer[index + 2];
        }
        else {
          Image::rgb565be_to_rgb888(*((uint16_t*)(buffer + index)), r1, g1, b1);
        }
        
        int xr, yr;
        xy_rotate(xr, yr, x, y, width, height, rotate_cw);
        
        int x1 = xr + offset_x;
        int y1 = yr + offset_y;

        color_func(x1, y1, r1, g1, b1, a, param);

        int index0 = (y1 * _width + x1) * 3;
        uint8_t r0 = _buffer[index0 + 0];
        uint8_t g0 = _buffer[index0 + 1];
        uint8_t b0 = _buffer[index0 + 2];

        uint8_t r2 = (uint8_t)(((uint16_t)r1 * a / 255) + ((uint16_t)r0 * (255 - a) / 255));
        uint8_t g2 = (uint8_t)(((uint16_t)g1 * a / 255) + ((uint16_t)g0 * (255 - a) / 255));
        uint8_t b2 = (uint8_t)(((uint16_t)b1 * a / 255) + ((uint16_t)b0 * (255 - a) / 255));
        setPixel(x1, y1, r2, g2, b2);

        int x2 = _width - (xr + offset_x) - 1;
        int y2 = (yr + offset_y);
        setPixel(x2, y2, r2, g2, b2);
      }
    }
  }
  else if (draw_mode == DRAW_MIRROR_ONLY_OFFSET) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int index = (y * width + x) * (bpp + has_alpha);
        uint8_t a = has_alpha ? buffer[index + bpp] : 255;
        if (a == 0) continue;
        
        uint8_t r1;
        uint8_t g1;
        uint8_t b1;

        if (bpp == 3) {
          r1 = buffer[index + 0];
          g1 = buffer[index + 1];
          b1 = buffer[index + 2];
        }
        else {
          Image::rgb565be_to_rgb888(*((uint16_t*)(buffer + index)), r1, g1, b1);
        }
        
        int xr, yr;
        xy_rotate(xr, yr, x, y, width, height, rotate_cw);
        
        int x1 = xr + offset_x;
        int y1 = yr + offset_y;

        color_func(x1, y1, r1, g1, b1, a, param);

        int index0 = (y1 * _width + x1) * 3;
        uint8_t r0 = _buffer[index0 + 0];
        uint8_t g0 = _buffer[index0 + 1];
        uint8_t b0 = _buffer[index0 + 2];

        uint8_t r2 = (uint8_t)(((uint16_t)r1 * a / 255) + ((uint16_t)r0 * (255 - a) / 255));
        uint8_t g2 = (uint8_t)(((uint16_t)g1 * a / 255) + ((uint16_t)g0 * (255 - a) / 255));
        uint8_t b2 = (uint8_t)(((uint16_t)b1 * a / 255) + ((uint16_t)b0 * (255 - a) / 255));
        setPixel(x1, y1, r2, g2, b2);

        int x2 = xr + _width - offset_x - width;
        int y2 = (yr + offset_y);
        setPixel(x2, y2, r2, g2, b2);
      }
    }
  }

  return true;
}


};
