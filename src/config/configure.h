#pragma once


#define HUB75_PANEL_RES_X                        64
#define HUB75_PANEL_RES_Y                        32
#define HUB75_PANEL_CHAIN                        2

// 주의: I2S 버스 사용시 HUB75와 충돌합니다.
#define NEOPIXEL_TYPE                            NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod>
