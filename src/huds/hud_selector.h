#pragma once
#include "hud_base.h"

#include <vector>


namespace toaster {

class HUDSelector : public HUDBase {
public:
  virtual void init();
  virtual void process(Adafruit_SSD1306& oled);
  virtual void release();
  virtual void pressKey(uint16_t key, uint8_t mode);

public:
  virtual void selectString(const std::vector<std::string>& strings, int select_index, std::function<void(int, void*)> on_select, void* on_select_param = nullptr) {
    _select_index = select_index;
    _strings = strings;
    _on_select = on_select;
    _on_select_param = on_select_param;
  }

protected:
  int _select_index{0};
  std::vector<std::string> _strings;
  std::function<void(int, void*)> _on_select;
  void* _on_select_param{nullptr};
};

extern HUDSelector hud_selector;

};
