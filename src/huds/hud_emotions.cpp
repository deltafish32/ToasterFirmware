#include "hud_emotions.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

std::vector<HUDEmotions> hud_emotions;
HUDEmotionDir hud_emotiondir;


void HUDEmotions::init() {
  HUDMenu::init();

  _menu_use_highlight = true;
  _menuHasHighlight = false;

  for (size_t i = 0; i < _menuData.size(); i++) {
    if (strcmp(_menuData[i].emotion.c_str(), Protogen.getEmotion()) == 0) {
      _menuHighlight = i;
      _menuHasHighlight = true;
      break;
    }
  }

}


void HUDEmotionDir::init() {
  HUDMenu::init();

  _menu_use_highlight = true;
  _menuHasHighlight = false;

  clearMenu();
  for (auto& it : hud_emotions) {
    addHUD(it.getGroupName(), &it);
  }

  for (size_t i = 0; i < _menuData.size(); i++) {
    if (strcmp(_menuData[i].text.c_str(), Protogen.getEmotionGroup()) == 0) {
      _menuHighlight = i;
      _menuHasHighlight = true;
      break;
    }
  }
}


};

