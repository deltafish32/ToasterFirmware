#include "hud_emotions.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

std::vector<HUDEmotions> hud_emotions;
HUDEmotionDir hud_emotiondir;


void HUDEmotions::init() {
  HUDMenu::init();

  _menu_use_highlight = true;
  _use_shuffle = (_menuData.size() > 1) ? true : false;

  refreshHighlight();
}


void HUDEmotions::refreshHighlight() {
  _menuHasHighlight = false;
  
  for (size_t i = 0; i < _menuData.size(); i++) {
    if (_menuData[i].type == MENU_FUNC && strcmp(_menuData[i].param.c_str(), Protogen.getEmotion()) == 0) {
      _menuHighlight = i;
      _menuHasHighlight = true;
      break;
    }
  }

  HUDMenu::refreshHighlight();
}


void HUDEmotionDir::init() {
  HUDMenu::init();

  _menu_use_highlight = true;
  _use_shuffle = true;

  clearMenu();
  for (auto& it : hud_emotions) {
    addHUD(it.getGroupName(), &it);
  }

  refreshHighlight();
}


void HUDEmotionDir::refreshHighlight() {
  highlightGroup(Protogen.getEmotionGroup());
}


void HUDEmotionDir::highlightGroup(const char* group) {
  _menuHasHighlight = false;

  for (size_t i = 0; i < _menuData.size(); i++) {
    if (strcmp(_menuData[i].text.c_str(), group) == 0) {
      _menuHighlight = i;
      _menuHasHighlight = true;
      break;
    }
  }

  HUDMenu::refreshHighlight();
}


void HUDEmotionDir::shuffle() {
  int index = Protogen.shuffleEmotion();
  if (index >= 0) {
    highlightGroup(Protogen.getEmotionGroup(index));
  }
}


};

