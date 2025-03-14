#include "effect_dino.h"
#include "lib/logger.h"
#include "protogen.h"


namespace toaster {


enum {
	STATE_INIT = 0,
	STATE_RUN,
	STATE_GAMEOVER,
};


static const uint8_t PROGMEM IMAGE_DINO[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
  0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x08, 0x06, 0x00, 0x00, 0x00, 0xaa, 0x69, 0x71,
  0xde, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72,
  0x65, 0x00, 0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x49, 0x6d, 0x61, 0x67, 0x65, 0x52, 0x65, 0x61,
  0x64, 0x79, 0x71, 0xc9, 0x65, 0x3c, 0x00, 0x00, 0x03, 0xc7, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda,
  0xec, 0x5b, 0x31, 0xb2, 0xd4, 0x30, 0x0c, 0xb5, 0x32, 0x29, 0xf8, 0x57, 0xa0, 0xa5, 0xf9, 0x0d,
  0x77, 0xd8, 0x8a, 0xe1, 0x14, 0x9c, 0x83, 0xfb, 0xc0, 0x41, 0x7e, 0xf5, 0x3b, 0x66, 0x28, 0x38,
  0x03, 0x05, 0x05, 0x05, 0x05, 0x74, 0x18, 0x67, 0x37, 0x09, 0x5a, 0xad, 0x24, 0x4b, 0xb1, 0x37,
  0xf1, 0xfe, 0x8d, 0x33, 0x9e, 0xc9, 0xfa, 0x5b, 0x59, 0x4b, 0x7e, 0x92, 0xa5, 0x97, 0xfd, 0x10,
  0x63, 0x0c, 0xb9, 0x06, 0x01, 0xd8, 0x49, 0x31, 0x44, 0x08, 0x86, 0x56, 0x2a, 0x7f, 0xcd, 0xd6,
  0x59, 0x17, 0xcf, 0x2d, 0x56, 0x52, 0xac, 0xa6, 0xfc, 0x38, 0x6f, 0xba, 0xe2, 0xd8, 0x8f, 0xd7,
  0x2a, 0x06, 0xf0, 0x2e, 0x56, 0x93, 0x6f, 0x61, 0xc7, 0x69, 0xeb, 0x57, 0xfc, 0x2e, 0x90, 0x8c,
  0x88, 0x76, 0x33, 0xce, 0x43, 0x27, 0xd4, 0xe0, 0x71, 0xb8, 0xf0, 0xa0, 0x71, 0x4c, 0x92, 0xc7,
  0x12, 0x93, 0xab, 0x03, 0xc0, 0xd9, 0xe7, 0xde, 0xbb, 0x3b, 0x12, 0x94, 0x0d, 0xf2, 0xd1, 0xe9,
  0xf7, 0x9c, 0xd2, 0x9e, 0x76, 0x92, 0x8f, 0x04, 0xc1, 0xf1, 0xfc, 0x99, 0xbd, 0x17, 0xf6, 0x78,
  0x2c, 0xa7, 0x8c, 0x43, 0xde, 0xa2, 0x6c, 0x14, 0xee, 0x73, 0xf2, 0xea, 0xb3, 0x3b, 0x6e, 0x61,
  0x53, 0xb0, 0xe1, 0x76, 0x1f, 0x77, 0x49, 0xb1, 0x25, 0xf2, 0x8a, 0xb2, 0x6a, 0x70, 0x9c, 0xdb,
  0x97, 0xf0, 0xe8, 0x90, 0xff, 0xbf, 0xd6, 0xc1, 0x17, 0xbc, 0x01, 0x4e, 0x53, 0xbe, 0x92, 0xfc,
  0x59, 0x0c, 0x10, 0x7d, 0x7c, 0x9c, 0x23, 0xc9, 0x0b, 0xc6, 0x80, 0x63, 0x08, 0x18, 0xd7, 0xd0,
  0x95, 0x2c, 0xdc, 0xef, 0x94, 0xa2, 0x3c, 0x14, 0xfa, 0xbb, 0x26, 0x0f, 0x9a, 0x41, 0x3a, 0xaf,
  0x62, 0xdc, 0xdc, 0x52, 0xf9, 0x02, 0x83, 0xe6, 0xcd, 0x92, 0x31, 0x48, 0xb7, 0xc2, 0xee, 0xd6,
  0x6f, 0x3f, 0xc2, 0x9b, 0xab, 0x64, 0x82, 0x37, 0x63, 0x84, 0xaf, 0xe1, 0xe1, 0x6a, 0xa9, 0xb0,
  0xa4, 0x44, 0x49, 0x9e, 0xb0, 0xc0, 0x4d, 0xf4, 0x09, 0xef, 0xc2, 0x37, 0x17, 0x42, 0x14, 0xc4,
  0x74, 0x39, 0x65, 0x4a, 0x76, 0xb5, 0x54, 0xbe, 0x1a, 0x42, 0x14, 0xc4, 0x00, 0x57, 0x0d, 0xd2,
  0xcc, 0xce, 0x9b, 0xc7, 0x7b, 0xe5, 0xe9, 0x31, 0x37, 0xcc, 0x85, 0x63, 0x12, 0x17, 0x4d, 0xa9,
  0xb2, 0x9a, 0x0a, 0xc7, 0xa0, 0xcb, 0x4a, 0x06, 0xc0, 0xb0, 0xd5, 0x2a, 0x3a, 0x4b, 0x05, 0xe8,
  0x95, 0xb7, 0x24, 0x3e, 0x31, 0xc4, 0x2a, 0x60, 0xe9, 0x3d, 0xfe, 0x4a, 0x13, 0x0e, 0xaf, 0xbf,
  0x5b, 0xe5, 0x77, 0x3e, 0x40, 0x09, 0x8e, 0xd3, 0xb5, 0x9a, 0x01, 0x6a, 0xf2, 0x01, 0xa1, 0xc1,
  0xd6, 0x85, 0x3b, 0x6f, 0x6b, 0xf2, 0x01, 0x55, 0xfc, 0x1e, 0x21, 0x09, 0x6a, 0xb8, 0x42, 0x2b,
  0x7c, 0x40, 0x75, 0xc5, 0xdc, 0x06, 0xc0, 0xbb, 0xb8, 0x84, 0xc5, 0x2d, 0x95, 0x37, 0xee, 0x3c,
  0x5c, 0xcd, 0x00, 0x96, 0x40, 0x95, 0x3b, 0xcf, 0x4b, 0xe5, 0x49, 0x7d, 0x0f, 0x5c, 0xfd, 0xcf,
  0x19, 0xa5, 0x04, 0x31, 0x9b, 0xf3, 0x01, 0x13, 0xcd, 0x6d, 0x34, 0x62, 0x75, 0xdf, 0x68, 0x82,
  0x0f, 0x20, 0x3b, 0x28, 0x91, 0x1b, 0x90, 0x61, 0x7a, 0xca, 0x10, 0xb0, 0x65, 0x29, 0xcc, 0xec,
  0x7a, 0xcc, 0xed, 0x3e, 0xe2, 0x16, 0xb3, 0x69, 0x73, 0xb3, 0x7c, 0x00, 0x4c, 0xb4, 0xbe, 0x8f,
  0x11, 0x9e, 0x65, 0xb5, 0x37, 0x44, 0xd6, 0xb8, 0xd0, 0x24, 0x1f, 0xc0, 0x28, 0x2d, 0x1a, 0xc9,
  0x13, 0x00, 0x39, 0x63, 0xf5, 0x16, 0x65, 0x96, 0xa4, 0xb1, 0xc2, 0x19, 0xcf, 0x45, 0xfc, 0x20,
  0xf8, 0xbb, 0xcb, 0xcf, 0x35, 0x24, 0x4c, 0xa5, 0x35, 0x35, 0xd6, 0x30, 0xde, 0x5b, 0x77, 0xb2,
  0x94, 0x13, 0xf0, 0x14, 0x3c, 0x56, 0xff, 0xb5, 0xcc, 0x85, 0x99, 0x1a, 0x00, 0x1f, 0x21, 0x52,
  0xb3, 0x5c, 0xcd, 0x54, 0x84, 0x2c, 0xa4, 0x69, 0xed, 0x6f, 0x99, 0x47, 0x5d, 0xc2, 0x62, 0xc8,
  0xf9, 0xc5, 0x08, 0x86, 0x3b, 0xbd, 0xe7, 0x14, 0xe0, 0x6a, 0x7b, 0x3a, 0x9f, 0x8e, 0x6b, 0xc8,
  0xe0, 0x14, 0xd1, 0x14, 0x58, 0x9a, 0xfc, 0x5c, 0x1a, 0x36, 0x06, 0x75, 0xd1, 0x9a, 0x41, 0x68,
  0x9c, 0xd0, 0x0c, 0x47, 0xef, 0x9d, 0xee, 0xb4, 0x28, 0xe0, 0x59, 0xe6, 0x8b, 0xc7, 0xa0, 0xf4,
  0xfa, 0x8a, 0xe3, 0xfb, 0xf0, 0xdf, 0xb8, 0xe7, 0x58, 0xea, 0x04, 0x0b, 0x11, 0xe2, 0x95, 0x29,
  0x4e, 0x85, 0xf1, 0xc2, 0x39, 0xf8, 0x53, 0xd6, 0x97, 0xa2, 0x83, 0xa2, 0x61, 0x33, 0x96, 0x58,
  0x89, 0x09, 0x67, 0x31, 0x20, 0xb7, 0x3b, 0x52, 0x0c, 0x90, 0xfc, 0x5b, 0x73, 0xa3, 0x56, 0x98,
  0x22, 0xb0, 0xfc, 0x48, 0xea, 0x25, 0xb7, 0x6e, 0x4b, 0x38, 0x7a, 0x7e, 0xe8, 0x94, 0xe6, 0x1e,
  0x20, 0xf0, 0x17, 0x93, 0x1e, 0x73, 0x73, 0x0f, 0xec, 0xf8, 0x96, 0x08, 0xb0, 0x46, 0x6a, 0x7a,
  0x12, 0x0c, 0x8a, 0x8c, 0xc3, 0x4f, 0xf8, 0xb8, 0xa4, 0xb9, 0x81, 0x94, 0x13, 0xa0, 0x67, 0x3c,
  0xed, 0x2e, 0x70, 0x0b, 0x8b, 0xf4, 0xb8, 0x0b, 0x1c, 0x7f, 0x07, 0x06, 0x0f, 0x8b, 0xca, 0xe1,
  0x56, 0x9b, 0x25, 0x0f, 0x48, 0x0a, 0xbf, 0x4a, 0xfd, 0x63, 0xba, 0xfd, 0x9b, 0xfa, 0xef, 0xd4,
  0x3f, 0xdf, 0xd5, 0x29, 0x80, 0x8e, 0xd3, 0xf7, 0xa9, 0x3f, 0xa7, 0xfe, 0x4b, 0xa8, 0x32, 0x5f,
  0xa7, 0xfe, 0x27, 0xf5, 0x9f, 0xd9, 0x72, 0x78, 0xa5, 0xc4, 0x24, 0x56, 0x7b, 0x1c, 0xff, 0x79,
  0x50, 0xf8, 0x7b, 0xea, 0x6f, 0xc9, 0x77, 0x7d, 0x4a, 0xfd, 0xc3, 0x1e, 0x04, 0x5b, 0x09, 0x70,
  0xc2, 0xb9, 0x3d, 0xdf, 0x4b, 0xe9, 0xac, 0x21, 0x27, 0x38, 0x93, 0xbf, 0x0c, 0x30, 0x43, 0x62,
  0xda, 0x48, 0x4f, 0x00, 0x3d, 0x0c, 0x20, 0x1d, 0xef, 0x03, 0xbe, 0xc7, 0x73, 0xd0, 0x7d, 0xf6,
  0x39, 0xda, 0xbc, 0x53, 0x19, 0xc0, 0x95, 0x4e, 0x5a, 0x26, 0x41, 0x0b, 0x75, 0xcc, 0x39, 0xb1,
  0xd5, 0x54, 0xdb, 0x2e, 0xd6, 0x8b, 0xcc, 0x83, 0xa4, 0xa8, 0xa6, 0xb4, 0x44, 0xf0, 0x01, 0x5c,
  0xa0, 0xae, 0x1d, 0x03, 0x68, 0x5c, 0x92, 0x46, 0xba, 0x73, 0x46, 0x10, 0x78, 0x2a, 0xfa, 0x53,
  0xf5, 0xb6, 0x10, 0x60, 0x81, 0xbd, 0xe6, 0x26, 0x0b, 0x10, 0xb0, 0x9f, 0x02, 0x4d, 0x21, 0x80,
  0xc2, 0xb8, 0xe4, 0x9f, 0x71, 0x04, 0x94, 0xb4, 0x1c, 0x08, 0x7b, 0x86, 0xe9, 0xc1, 0xf4, 0x56,
  0x20, 0x74, 0x97, 0x3a, 0x6e, 0xf4, 0x88, 0xc6, 0x52, 0xe8, 0x2d, 0x8a, 0x9b, 0xe6, 0x4e, 0x81,
  0x0a, 0xe7, 0xbd, 0x15, 0xfe, 0xad, 0x05, 0xc4, 0x5e, 0x3d, 0xef, 0x73, 0xf4, 0x8c, 0x36, 0x8e,
  0x9e, 0x83, 0x95, 0x6e, 0x2d, 0x1e, 0x2c, 0xab, 0x06, 0x9d, 0xef, 0xa5, 0xb0, 0xd2, 0x83, 0x31,
  0xda, 0x4c, 0x84, 0x72, 0xc9, 0x0e, 0xad, 0x34, 0x5e, 0x08, 0x02, 0xf6, 0x3c, 0x40, 0xcd, 0x03,
  0xb8, 0xdd, 0xb7, 0x66, 0x85, 0xb7, 0xc2, 0x24, 0xed, 0x84, 0xc8, 0x9d, 0xb7, 0x7f, 0x02, 0x0c,
  0x00, 0x09, 0x80, 0x56, 0xe7, 0xa8, 0x47, 0xf7, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
  0x44, 0xae, 0x42, 0x60, 0x82,
};


static const int DINO_W = 16;
static const int DINO_H = 16;

static const int DINO_JUMP_SX = 0;
static const int DINO_JUMP_SY = 0;

static const int DINO_DED_SX = 16;
static const int DINO_DED_SY = 0;

static const int DINO_RUN_SX[] = {0, 16, };
static const int DINO_RUN_SY[] = {16, 16, };

typedef struct _CACTUS {
  uint8_t w;
  uint8_t h;
  uint8_t sx;
  uint8_t sy;
} CACTUS;

typedef struct _TILE {
  uint8_t sx;
  uint8_t sy;
  uint16_t rarity;
} TILE;

static const CACTUS CACTUS_DATA[] = {
  { 8, 16, 32, 0, },
  { 8, 16, 40, 0, },
  { 16, 16, 48, 0, },
  { 16, 16, 32, 16, },
};

static const int CACTUS_TYPE_COUNT = sizeof(CACTUS_DATA) / sizeof(CACTUS_DATA[0]);

static const int TILE_W = 8;
static const int TILE_H = 8;

static const TILE TILE_DATA[] = {
  { 32, 32, 10, },
  { 40, 32, 100, },
  { 48, 32, 100, },
  { 56, 32, 100, },
  { 32, 40, 100, },
  { 40, 40, 100, },
  { 48, 40, 10, },
  { 56, 40, 10, },
};

static const int TILE_TYPE_COUNT = sizeof(TILE_DATA) / sizeof(TILE_DATA[0]);

static const int SCORE_W = 3;
static const int SCORE_H = 5;
static const int SCORE_SX = 0;
static const int SCORE_SY = 32;

static const int GAMEOVER_W = 24;
static const int GAMEOVER_H = 16;
static const int GAMEOVER_SX = 0;
static const int GAMEOVER_SY = 48;



// g = 9.8, v = 2.0, fps = 180
static const float JUMP_LUT[] = {
  0.052994f, 0.104547f, 0.154655f, 0.203322f, 0.250545f, 0.296327f, 0.340664f, 0.383561f, 0.425012f, 0.465023f,
  0.503589f, 0.540715f, 0.576400f, 0.610640f, 0.643435f, 0.674789f, 0.704703f, 0.733172f, 0.760201f, 0.785784f,
  0.809927f, 0.832625f, 0.853883f, 0.873695f, 0.892067f, 0.908994f, 0.924480f, 0.938526f, 0.951127f, 0.962283f,
  0.971998f, 0.980273f, 0.987103f, 0.992493f, 0.996437f, 0.998936f, 0.999999f, 0.999614f, 0.997792f, 0.994525f,
  0.989812f, 0.983660f, 0.976062f, 0.967024f, 0.956545f, 0.944622f, 0.931253f, 0.916444f, 0.900194f, 0.882499f,
  0.863364f, 0.842784f, 0.820759f, 0.797298f, 0.772391f, 0.746040f, 0.718248f, 0.689012f, 0.658334f, 0.626212f,
  0.592649f, 0.557646f, 0.521198f, 0.483304f, 0.443975f, 0.403196f, 0.360977f, 0.317317f, 0.272213f, 0.225667f,
  0.177677f, 0.128246f, 0.077370f, 0.025054f,
};

static const int JUMP_FRAMES = sizeof(JUMP_LUT) / sizeof(JUMP_LUT[0]);

static const timer_ms_t MIN_BLOCK_TIME_MS = 1500;
static const timer_ms_t MAX_BLOCK_TIME_MS = 3000;
static const timer_ms_t RESTART_TIME_MS = 1000;


bool DinoCactus::hittest(int x, int y) {
  int dino_x_expand = 2;
  int dino_y = y + 8;
  return _hitable
      && (x + dino_x_expand >= _x - CACTUS_DATA[_type].w / 2 && x - dino_x_expand < _x + CACTUS_DATA[_type].w / 2)
      && (dino_y - (_y + _height * 1 / 4) > 0);
}


void DinoCactus::draw(Display& display, EffectDino& dino) {
  display.draw_image_newcolor_ex(dino._image_dino, dino._colorFunc, 0, DRAW_MIRROR, 
    _x - CACTUS_DATA[_type].h / 2, _y, CACTUS_DATA[_type].w, CACTUS_DATA[_type].h, CACTUS_DATA[_type].sx, CACTUS_DATA[_type].sy);
}


void EffectDino::init(Display& display) {
  Effect::init(display);

  _state = STATE_INIT;
  _frame = 0;
  _jumping = false;
  _jump_frame = 0;
  _score = 0;
  _tile_x = 0;
  _next_block_time = Timer::get_millis() + MAX_BLOCK_TIME_MS;
  _prev_static_mode = getStaticMode();
  setStaticMode(true);

  _objects.clear();
  generateTile(true);
  
  _image_dino = new Image(Image::IMAGE_PNG, IMAGE_DINO, sizeof(IMAGE_DINO) / sizeof(IMAGE_DINO[0]), true);
}


void EffectDino::process(Display& display) {
  processGames();
  drawGames(display);

  ++_frame;
}


void EffectDino::release(Display& display) {
  if (_image_dino != nullptr) {
    delete _image_dino;
    _image_dino = nullptr;
  }

  setStaticMode(_prev_static_mode);

  Effect::release(display);
}


int EffectDino::getDinoX() {
  return 22;
}


int EffectDino::getDinoY() {
  return 16 - (_jumping ? (int)(JUMP_LUT[_jump_frame] * 16) : 0);
}


void EffectDino::drawGames(Display& display) {
  const int dino_x = getDinoX();
  const int dino_y = getDinoY();
  int dino_sx = 0;
  int dino_sy = 0;
  
  for (int i = 0; i < _tiles.size(); i++) {
    int tile_type = _tiles[i];
    display.draw_image_newcolor_ex(_image_dino, _colorFunc, 0, DRAW_MIRROR, 
      0 - _tile_x + i * TILE_W, HUB75_PANEL_RES_Y - TILE_H, TILE_W, TILE_H, TILE_DATA[tile_type].sx, TILE_DATA[tile_type].sy);
  }

  for (const auto& obj : _objects) {
    obj->draw(display, *this);
  }

  switch (_state) {
  case STATE_INIT:
    dino_sx = DINO_JUMP_SX;
    dino_sy = DINO_JUMP_SY;
    break;
  case STATE_RUN: {
      int run_frame = (Timer::get_millis() - _jump_time) / 250 % 2;
      dino_sx = _jumping ? DINO_JUMP_SX : DINO_RUN_SX[run_frame];
      dino_sy = _jumping ? DINO_JUMP_SY : DINO_RUN_SY[run_frame];
    }
    break;
  case STATE_GAMEOVER:
    dino_sx = DINO_DED_SX;
    dino_sy = DINO_DED_SY;

    display.draw_image_newcolor_ex(_image_dino, _colorFunc, 0, DRAW_SINGLE, 18, 0, GAMEOVER_W, GAMEOVER_H, GAMEOVER_SX, GAMEOVER_SY);
    display.draw_image_newcolor_ex(_image_dino, _colorFunc, 0, DRAW_SINGLE, 66, 0, GAMEOVER_W, GAMEOVER_H, GAMEOVER_SX, GAMEOVER_SY);
    break;
  }
  
  display.draw_image_newcolor_ex(_image_dino, _colorFunc, 0, DRAW_MIRROR, dino_x, dino_y, DINO_W, DINO_H, dino_sx, dino_sy);

  printScore(display, _score);
}


void EffectDino::processGames() {
  if (_jumping == false && Protogen._boopsensor.isBoop()) {
    _jumping = true;
    _jump_frame = 0;
  }

  switch (_state) {
  case STATE_INIT:
    if (_jumping) {
      _state = STATE_RUN;
      _jump_time = Timer::get_millis();
      _next_block_time = Timer::get_millis() + MIN_BLOCK_TIME_MS;
      _objects.clear();
      _frame = 0;
      _score = 0;
      setStaticMode(false);
    }
    break;
  case STATE_RUN:
    for (const auto& obj : _objects) {
      if (obj->hittest(getDinoX() + DINO_W / 2, getDinoY())) {
        _state = STATE_GAMEOVER;
        _ded_time = Timer::get_millis();
        setStaticMode(true);
      }
    }

    if (Timer::get_millis() >= _next_block_time) {
      _next_block_time = Timer::get_millis() + Random::random(MAX_BLOCK_TIME_MS - MIN_BLOCK_TIME_MS) + MIN_BLOCK_TIME_MS;
      generateCactus();
    }

    if ( _jumping) {
      ++_jump_frame;
      if (_jump_frame >= JUMP_FRAMES) {
        _jumping = false;
        _jump_time = Timer::get_millis();
      }
    }

    if (_frame % 2 == 0) {
      ++_tile_x;
      if (_tile_x >= TILE_W) {
        _tile_x = 0;
        generateTile(false);
      }

      for (auto it = _objects.begin(); it != _objects.end(); ) {
        (*it)->process();
        if (!(*it)->_valid) {
          _objects.erase(it);
        }
        else {
          ++it;
        }
      }
    }

    if (_frame % 6 == 0) {
      ++_score;

      if (_score % 100 == 0) {
        _score_highlight = 8;
      }
    }

    if (_frame % 15 == 0 && _score_highlight > 0) {
      --_score_highlight;
    }
    break;
  case STATE_GAMEOVER:
    if (_jumping && Timer::get_millis() - _ded_time >= RESTART_TIME_MS) {
      _state = STATE_RUN;
      _jump_time = Timer::get_millis();
      _objects.clear();
      _next_block_time = _jump_time + MAX_BLOCK_TIME_MS;
      _frame = 0;
      _score = 0;
      setStaticMode(false);
    }
    break;
  }
}


void EffectDino::generateCactus() {
  int type = Random::random(CACTUS_TYPE_COUNT);
  auto cactus = new DinoCactus(type, HUB75_PANEL_RES_X + CACTUS_DATA[type].w / 2, 16, CACTUS_DATA[type].w, CACTUS_DATA[type].h, true);
  _objects.push_back(cactus);
}


void EffectDino::generateTile(bool init) {
  const int TILE_COUNT = HUB75_PANEL_RES_X / TILE_W + 1;

  static std::random_device rd;
  static std::mt19937 gen(_rd());

  static int tile_weights[TILE_TYPE_COUNT];
  if (tile_weights[0] == 0) {
    for (int i = 0; i < TILE_TYPE_COUNT; i++) {
      tile_weights[i] = TILE_DATA[i].rarity;
    }
  }

  static std::discrete_distribution<int> dist(tile_weights, tile_weights + TILE_TYPE_COUNT);

  if (init) {
    _tiles.clear();
    _tiles.assign(TILE_COUNT, 0);

    for (int i = 0; i < TILE_COUNT; i++) {
      _tiles[i] = dist(gen);
    }
  }
  else {
    memmove(&_tiles[0], &_tiles[1], sizeof(int) * (TILE_COUNT - 1));
    _tiles.back() = dist(gen);
  }
}


void EffectDino::printScore(Display& display, int score) {
  int display_score = _score_highlight > 0 ? (score / 100 * 100) : score;

  if (_score_highlight % 2 != 0) {
    return;
  }

  int digit = 0;
  do {
    int n = display_score % 10;
    display.draw_image_newcolor_ex(_image_dino, _colorFunc, 0, DRAW_SINGLE, HUB75_PANEL_RES_X - 1 - (digit + 1) * (SCORE_W + 1), 0, SCORE_W, SCORE_H, SCORE_SX + (n * SCORE_W), SCORE_SY);
    display.draw_image_newcolor_ex(_image_dino, _colorFunc, 0, DRAW_SINGLE, HUB75_PANEL_RES_X - 1 + 48 - (digit + 1) * (SCORE_W + 1), 0, SCORE_W, SCORE_H, SCORE_SX + (n * SCORE_W), SCORE_SY);
    display_score /= 10;
    ++digit;
  } while (display_score > 0);
}


};
