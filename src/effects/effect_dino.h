#pragma once
#include "effect_base.h"


namespace toaster {


class EffectDino;

class DinoObject {
public:
  DinoObject(int x, int y, int w, int h, bool hitable) : _x(x), _y(y), _width(w), _height(h), _hitable(hitable), _valid(true) {
  }

  virtual void process() {
    _x -= 1;
    if (_x + _width < 0) {
      _valid = false;
    }
  }

  virtual void draw(Display& display, EffectDino& dino) {}
  virtual bool hittest(int x, int y) { return false; }

public:
  bool _valid{true};
  bool _hitable{false};
  int _x{0};
  int _y{0};
  int _width{0};
  int _height{0};
};


class DinoCactus : public DinoObject {
public:
  DinoCactus(int type, int x, int y, int w, int h, bool hitable) : DinoObject(x, y, w, h, hitable), _type(type) {
  }

  virtual void draw(Display& display, EffectDino& dino);
  virtual bool hittest(int x, int y);

public:
  int _type;
};


class EffectDino : public FixedEffect {
public:
  EffectDino(const char* name) : FixedEffect(name) {
  }
  
  virtual void init(Display& display);
  virtual void process(Display& display);
  virtual void release(Display& display);

  virtual bool isGame() const {
    return true;
  }

protected:
  int _state{0};
  int _frame{0};
  bool _jumping{false};
  int _jump_frame{0};
  int _score{0};
  int _score_highlight{0};
  timer_ms_t _jump_time{0};
  timer_ms_t _next_block_time{0};
  timer_ms_t _ded_time{0};
  timer_ms_t _score_highlight_time{0};
  std::vector<DinoObject*> _objects;
  std::vector<int> _tiles;
  int _tile_x{0};
  bool _prev_static_mode{false};
  
public:
  Image* _image_dino{nullptr,};

public:
  int getDinoX();
  int getDinoY();

protected:
  void drawGames(Display& display);
  void processGames();
  void generateCactus();
  void generateTile(bool init);
  void printScore(Display& display, int score);
};

};
