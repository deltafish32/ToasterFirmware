#pragma once
#include "hud_menu.h"


namespace toaster {

class HUDEmotions : public HUDMenu {
public:
  HUDEmotions() {}
  HUDEmotions(const char* group) : _group(group) {
  }

  virtual void init();

  const char* getGroupName() const {
    return _group.c_str();
  }

protected:
  std::string _group;
};

class HUDEmotionDir : public HUDMenu {
public:
  virtual void init();

};

extern std::vector<HUDEmotions> hud_emotions;
extern HUDEmotionDir hud_emotiondir;
 
};
