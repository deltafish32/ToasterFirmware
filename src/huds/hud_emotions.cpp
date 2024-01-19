#include "hud_emotions.h"


#include "huds.h"
#include "protogen.h"


namespace toaster {

HUDEmotions hud_emotions;

HUDEmotions* HUDEmotions::_pthis = nullptr;


void HUDEmotions::init() {
  HUDMenu::init();

  // setMenuData(menu_emotion, menu_emotion_count);
}

};

