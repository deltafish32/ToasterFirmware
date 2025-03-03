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
  _menuHasHighlight = false;

  for (size_t i = 0; i < _menuData.size(); i++) {
    if (strcmp(_menuData[i].text.c_str(), Protogen.getEmotionGroup()) == 0) {
      _menuHighlight = i;
      _menuHasHighlight = true;
      break;
    }
  }

  HUDMenu::refreshHighlight();
}


void HUDEmotionDir::shuffle() {
  Protogen.shuffleEmotion();
  refreshHighlight();
}


};

