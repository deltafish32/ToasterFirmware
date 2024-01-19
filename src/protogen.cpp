#include "protogen.h"
#include "effects/effects.h"

#include "huds/hud.h"
#include "huds/hud_emotions.h"
#include "lib/logger.h"
#include "lib/strutil.h"
#include <map>


namespace toaster {

static const char* TAG = "Protogen";


Toaster Protogen;


static const int ADC_MAX = 4095;


Toaster::Toaster() {
}


bool Toaster::begin() {
  if (_init) {
    return true;
  }

  _serialdebug.begin();

  if (!SPIFFS.begin()) {
    TF_LOG(TAG, "begin: SPIFFS mount failed");
    return false;
  }

  YamlNodeArray yaml = YamlNodeArray::fromFile("/config.yaml", SPIFFS);

  if (yaml.isError()) {
    TF_LOG(TAG, "begin: config.yaml does not exist or format error (%s)", yaml.getLastError());
    return false;
  }

  int yaml_version = atoi(yaml.getString("version", "0").c_str());
  if (yaml_version != 1) {
    TF_LOG(TAG, "begin: config.yaml version mismatch (%d, required %d)", yaml_version, 1);
    return false;
  }

  // TF_LOG(TAG, "config.yaml\n\n%s", yaml.toString().c_str());
  
  if (!loadHub75(yaml)) {
    TF_LOG(TAG, "begin: Display begin failed");
    return false;
  }

  if (!loadSidePanel(yaml)) {
    TF_LOG(TAG, "begin: Side Display begin failed");
    return false;
  }

  if (!loadHUD(yaml)) {
    TF_LOG(TAG, "begin: HUD begin failed");
  }

  loadPhotoresistor(yaml);
  loadRemote(yaml);
  loadPersonality(yaml);
  loadEmotions(yaml);
  
  _interruptSemaphore = xSemaphoreCreateBinary();
  syncUnlock();

  uint32_t hub75_fps = atoi(yaml.getString("hardware:hub75:fps", "60").c_str());
  if (!Worker::begin(hub75_fps)) {
    return false;
  }

  _init = true;

  xTaskCreatePinnedToCore([](void* param) {
    auto pthis = (Toaster*)param;
    while (1) {
      pthis->_hud.loop();
      pthis->_i2c_espnow.loop();

      delay(1);
    }
    }, "hud", 8192, this, 1, nullptr, PRO_CPU_NUM);

  return true;
}


void Toaster::loop() {
  Worker::loop();

  syncLock();

  _ir_remote.loop();

  syncUnlock();
}


void Toaster::work() {
  syncLock();

  _serialdebug.loop();

  _display.beginDraw();
  _display.clear();

  Effect::setDirty();
  for (int i = 0; i < 3; i++) {
    if (_effect[i] != nullptr) {
      _effect[i]->process(_display);
    }
  }
  
  _display.endDraw();

  if (_side_display_use) {
    _side_display.beginDraw();
    _side_display.clear();
    
    if (_side_effect != nullptr) {
      _side_effect->process(_side_display);
    }
    
    _side_display.endDraw();
  }

  if (_photoresistor_use) {
    _photoresistor.loop();
    int light = _photoresistor.getValue();
    if (abs(_pr_recent - light) >= _pt_hys) {
      _pr_recent = light;
      refreshAutoBrightness();
    }
  }
  
  syncUnlock();
}


void Toaster::workPerSecond() {
  int hud_fps = _hud.getRecentFPS();

  TF_LOG(TAG, "FPS: %d, HUD FPS: %d, heap: %d", getRecentFPS(), hud_fps, ESP.getFreeHeap());
}


bool Toaster::setEmotion(const char* emotion) {
  for (const auto& it : _emotions) {
    if (it.name == emotion) {
      setEffect(it.mouth.c_str(), it.nose.c_str(), it.eyes.c_str());
      setSideEffect(it.side.c_str());
      return true;
    }
  }

  return false;
}


void Toaster::setEffect(const char* new_effect1, const char* new_effect2, const char* new_effect3) {
  const char* new_effects[] = {new_effect1, new_effect2, new_effect3};

  for (int i = 0; i < 3; i++) {
    const char* prev_effect = (_effect[i] != nullptr) ? _effect[i]->getName() : "";
    const char* new_effect = new_effects[i];

    if (strcasecmp(prev_effect, new_effect) != 0) {
      if (_effect[i] != nullptr) {
        _effect[i]->release(_display);
      }

      _effect[i] = Effect::find(new_effect, i);

      if (_effect[i] != nullptr) {
        _effect[i]->init(_display);
      }
    }
  }
}


void Toaster::setSideEffect(const char* new_effect1) {
  if (_side_display_use == false) {
    return;
  }

  const char* prev_effect = (_side_effect != nullptr) ? _side_effect->getName() : "";
  const char* new_effect = (new_effect1 != nullptr) ? new_effect1 : "";

  if (strcasecmp(prev_effect, new_effect) != 0) {
    if (_side_effect != nullptr) {
      _side_effect->release(_side_display);
    }

    _side_effect = Effect::find(new_effect, 3);

    if (_side_effect != nullptr) {
      _side_effect->init(_side_display);
    }
  }
}


void Toaster::toast() {
  TF_LOG(TAG, "toast: Sorry, this feature is not supported.");
}


void Toaster::syncLock(TickType_t xBlockTime) {
  uint32_t current_core = xPortGetCoreID();

  auto lock = [this, xBlockTime]() {
    if (_interruptSemaphore != nullptr) {
      xSemaphoreTake(_interruptSemaphore, xBlockTime);
    }
  };

  if (_sync_count <= 0) {
    lock();

    ++_sync_count;
    _sync_core = current_core;
  }
  else {
    if (_sync_core == current_core) {
      ++_sync_count;
    }
    else {
      lock();
    }
  }
}


void Toaster::syncUnlock() {
  uint32_t current_core = xPortGetCoreID();

  auto unlock = [this]() {
    if (_interruptSemaphore != nullptr) {
      xSemaphoreGive(_interruptSemaphore);
    }
  };

  if (_sync_count > 0 && _sync_core == current_core) {
    --_sync_count;
    if (_sync_count <= 0) {
      unlock();
    }
  }
  else {
    unlock();
  }
}


void Toaster::refreshAutoBrightness() {
  float new_brightness;

  if (!_pr_auto.getValue(_pr_recent, new_brightness, true)) {
    return;
  }

  uint8_t new_brightness_8 = (uint8_t)(new_brightness * 255.0f);
  _display.setBrightness(new_brightness_8);

  if (_side_display_use) {
    _side_display.setBrightness(new_brightness_8);
  }
}


bool Toaster::loadHub75(const YamlNodeArray& yaml) {
  HUB75_I2S_CFG::i2s_pins hub75_pins;

  hub75_pins.r1 = atoi(yaml.getString("hardware:hub75:r1_pin", "25").c_str());
  hub75_pins.g1 = atoi(yaml.getString("hardware:hub75:g1_pin", "26").c_str());
  hub75_pins.b1 = atoi(yaml.getString("hardware:hub75:b1_pin", "27").c_str());
  hub75_pins.r2 = atoi(yaml.getString("hardware:hub75:r2_pin", "14").c_str());
  hub75_pins.g2 = atoi(yaml.getString("hardware:hub75:g2_pin", "12").c_str());
  hub75_pins.b2 = atoi(yaml.getString("hardware:hub75:b2_pin", "13").c_str());

  hub75_pins.a = atoi(yaml.getString("hardware:hub75:a_pin", "23").c_str());
  hub75_pins.b = atoi(yaml.getString("hardware:hub75:b_pin", "19").c_str());
  hub75_pins.c = atoi(yaml.getString("hardware:hub75:c_pin", "5").c_str());
  hub75_pins.d = atoi(yaml.getString("hardware:hub75:d_pin", "33").c_str());
  hub75_pins.e = atoi(yaml.getString("hardware:hub75:e_pin", "32").c_str());
  
  hub75_pins.lat = atoi(yaml.getString("hardware:hub75:lat_pin", "4").c_str());
  hub75_pins.oe = atoi(yaml.getString("hardware:hub75:oe_pin", "15").c_str());
  hub75_pins.clk = atoi(yaml.getString("hardware:hub75:clk_pin", "18").c_str());

  if (!_display.begin(HUB75_PANEL_RES_X, HUB75_PANEL_RES_Y, HUB75_PANEL_CHAIN, hub75_pins)) {
    return false;
  }

  float default_brightness = atof(yaml.getString("hardware:hub75:default_brightness", "1.0").c_str());
  _display.setBrightness(default_brightness);

  return true;
}


bool Toaster::loadSidePanel(const YamlNodeArray& yaml) {
  _side_display_use = yaml.isKeysExist("hardware:side_panel");
  if (_side_display_use) {
    int side_panel_num_leds = atoi(yaml.getString("hardware:side_panel:num_leds", "1").c_str());
    int side_panel_pin = atoi(yaml.getString("hardware:side_panel:pin", "17").c_str());
    if (!_side_display.begin(side_panel_num_leds, side_panel_pin)) {
      return false;
    }
  }

  return true;
}


bool Toaster::loadHUD(const YamlNodeArray& yaml) {
  _hud_use = yaml.isKeysExist("hardware:hud");
  if (_hud_use) {
    uint8_t hud_i2c = parse_hex(yaml.getString("hardware:hud:i2c", "0x3C").c_str());
    uint32_t hud_fps = atoi(yaml.getString("hardware:hud:fps", "30").c_str());
    if (!_hud.begin(hud_i2c, hud_fps)) {
      return false;
    }
  }

  return true;
}


bool Toaster::loadPhotoresistor(const YamlNodeArray& yaml) {
  _photoresistor_use = yaml.isKeysExist("hardware:photoresistor");
  if (_photoresistor_use) {
    int pr_pin = atoi(yaml.getString("hardware:photoresistor:pin", "34").c_str());
    float pr_alpha = atof(yaml.getString("hardware:photoresistor:alpha", "0.98").c_str());
    float pr_alpha_init = atof(yaml.getString("hardware:photoresistor:alpha_init", "0.9").c_str());
    float hys = atof(yaml.getString("hardware:photoresistor:hysteresis", "0.05").c_str());
    _pt_hys = (int)(hys * ADC_MAX);

    _photoresistor.begin(pr_pin, pr_alpha, pr_alpha_init);
    _pr_recent = _photoresistor.getValue();
  
    auto cal_points = yaml.findKeys("hardware:photoresistor:calibration:points");
    if (cal_points == nullptr || cal_points->asObjects().size() < 2) {
      _pr_auto.pushDataPoint({1024, 0.25f});
      _pr_auto.pushDataPoint({3200, 1.0f});
    }
    else {
      for (const auto& it : cal_points->asObjects()) {
        if (it.isString() == false) continue;

        std::string left;
        std::string right;
        parse_separate(it.asString().c_str(), " -> ", left, right);
        float sensor = atof(left.c_str());
        float brightness = atof(right.c_str());
        _pr_auto.pushDataPoint({(int)(sensor * ADC_MAX), brightness});
      }
    }

    refreshAutoBrightness();
  }

  return true;
}


bool Toaster::loadRemote(const YamlNodeArray& yaml) {
  _espnow_use = yaml.isKeysExist("hardware:i2c_esp_now");
  if (_espnow_use) {
    uint8_t espnow_i2c = parse_hex(yaml.getString("hardware:i2c_esp_now:i2c", "0x15").c_str());
    _i2c_espnow.begin(espnow_i2c);
  }

  _ir_remote_use = yaml.isKeysExist("hardware:ir_remote");
  if (_ir_remote_use) {
    int ir_remote_pin = atoi(yaml.getString("hardware:ir_remote:pin", "35").c_str());
    bool ir_remote_debug = parse_bool(yaml.getString("hardware:ir_remote:debug", "false").c_str());
    _ir_remote.begin(ir_remote_pin, ir_remote_debug);

    loadNEC(yaml);
  }

  return true;
}


bool Toaster::loadPersonality(const YamlNodeArray& yaml) {
  std::map<std::string, std::string> color_map;

  std::string color_eyes_str = yaml.getString("my_protogen:color_eyes", "#00ff00");
  std::string color_nose_str = yaml.getString("my_protogen:color_nose", "color_eyes");
  std::string color_mouth_str = yaml.getString("my_protogen:color_mouth", "color_eyes");
  std::string color_side_str = yaml.getString("my_protogen:color_side", "color_eyes");

  color_map.insert({"color_eyes", color_eyes_str});
  color_map.insert({"color_nose", color_nose_str});
  color_map.insert({"color_mouth", color_mouth_str});
  color_map.insert({"color_side", color_side_str});

  if (!color_map.empty()) {
    auto set_color = [&color_map](const char* name, uint8_t index, uint32_t def_color) -> bool {
      std::string color_ref = name;

      for (int i = 0; i < 4; i++) {
        auto find = color_map.find(color_ref);
        if (find == color_map.end()) {
          break;
        }

        if (find->second[0] != 'c') {
          uint32_t color = parse_hex(find->second.c_str());
          Effect::setPersonalColor(index, color);
          return true;
        }

        color_ref = find->second;
      }

      Effect::setPersonalColor(index, def_color);
      return false;
    };

    set_color("color_eyes", Effect::PC_EYES - Effect::PC_EYES, 0x00ff00);
    set_color("color_nose", Effect::PC_NOSE - Effect::PC_EYES, 0x00ff00);
    set_color("color_mouth", Effect::PC_MOUTH - Effect::PC_EYES, 0x00ff00);
    set_color("color_side", Effect::PC_SIDE - Effect::PC_EYES, 0x00ff00);
  }

  std::string color_mode = yaml.getString("my_protogen:color_mode", "personal");
  PROTOGEN_COLOR_MODE pcm = PCM_PERSONAL;
  if (color_mode == "original") {
    pcm = PCM_ORIGINAL;
  }
  else if (color_mode == "personal") {
    pcm = PCM_PERSONAL;
  }
  else if (color_mode == "rainbow_single") {
    pcm = PCM_RAINBOW_SINGLE;
  }
  else if (color_mode == "rainbow") {
    pcm = PCM_RAINBOW;
  }
  else if (color_mode == "gradation") {
    pcm = PCM_GRADATION;
  }
  
  Effect::setColorMode(pcm);
  
  std::string eyes_default = yaml.getString("my_protogen:eyes_default", "eyes_normal");
  std::string nose_default = yaml.getString("my_protogen:nose_default", "nose_default");
  std::string mouth_default = yaml.getString("my_protogen:mouth_default", "mouth_default");
  std::string side_default = yaml.getString("my_protogen:side_default", "side_default");

  setEffect(mouth_default.c_str(), nose_default.c_str(), eyes_default.c_str());
  setSideEffect(side_default.c_str());

  uint32_t rainbow_single_speed = parse_time_str(yaml.getString("my_protogen:rainbow_single_speed", "20ms").c_str());
  Effect::setRainbowSingleSpeed(rainbow_single_speed);

  uint32_t rainbow_speed = parse_time_str(yaml.getString("my_protogen:rainbow_speed", "10ms").c_str());
  uint32_t rainbow_transition_pixels = atoi(yaml.getString("my_protogen:rainbow_transition_pixels", "4").c_str());
  Effect::setRainbowSpeed(rainbow_speed, rainbow_transition_pixels);

  auto g_points = yaml.findKeys("my_protogen:gradation_points");
  if (g_points == nullptr || g_points->asObjects().size() < 2) {
    Effect::setGradationPoint(0.0f, 0, 255, 0);
    Effect::setGradationPoint(1.0f, 0, 0, 255);
  }
  else {
    for (const auto& it : g_points->asObjects()) {
      if (it.isString() == false) continue;

      std::string left;
      std::string right;
      parse_separate(it.asString().c_str(), " -> ", left, right);
      uint32_t color = parse_hex(right.c_str());
      Effect::setGradationPoint(atof(left.c_str()), (color >> 0) & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff);
    }
  }

  return true;
}


bool Toaster::loadEmotions(const YamlNodeArray& yaml) {
  auto emotions = yaml.findKeys("emotions");
  if (emotions == nullptr || emotions->isObject() == false || emotions->asObjects().empty()) {
    return false;
  }

  bool is_default_loaded = false;
  
  for (const auto& it : emotions->asObjects()) {
    auto& emotion = it.asObjects();
    
    auto name = emotion.findKeys("name");
    if (name == nullptr) continue;

    auto eyes = emotion.findKeys("eyes");
    if (eyes == nullptr) continue;

    auto nose = emotion.findKeys("nose");
    if (nose == nullptr) continue;

    auto mouth = emotion.findKeys("mouth");
    if (mouth == nullptr) continue;

    auto side = emotion.findKeys("side");
    if (side == nullptr) continue;

    if (name->asString() == "default") {
      _emotion_default.name = name->asString();
      _emotion_default.eyes = eyes->asString();
      _emotion_default.nose = nose->asString();
      _emotion_default.mouth = mouth->asString();
      _emotion_default.side = side->asString();
      is_default_loaded = true;
      break;
    }
  }

  if (is_default_loaded == false) {
    return false;
  }

  _emotions.push_back({"blank", "", "", "blank", "blank"});
  _emotions.push_back({"white", "", "", "white", "white"});
  // _emotions.push_back({"festive", "", "", "festive", "side_festive"});
  _emotions.push_back({"festive", "", "", "festive_face", "side_rainbow"});
  _emotions.push_back({"align", "", "", "blank", "side_align"});
  
  for (const auto& it : emotions->asObjects()) {
    if (it.isObject() == false) {
      continue;
    }

    auto& emotion = it.asObjects();
    
    auto name = emotion.findKeys("name");
    if (name == nullptr) continue;

    auto eyes = emotion.findKeys("eyes");
    auto nose = emotion.findKeys("nose");
    auto mouth = emotion.findKeys("mouth");
    auto side = emotion.findKeys("side");

    if (name->asString() == "default") {
      continue;
    }
    
    _emotions.push_back({name->asString(), 
      (eyes == nullptr) ? _emotion_default.eyes : eyes->asString(), 
      (nose == nullptr) ? _emotion_default.nose : nose->asString(), 
      (mouth == nullptr) ? _emotion_default.mouth : mouth->asString(), 
      (side == nullptr) ? _emotion_default.side : side->asString()});
    HUDEmotions::addEmotion(name->asString().c_str(), name->asString().c_str());
  }
  
  HUDEmotions::addEmotion("~(^w^)~", "festive");
  HUDEmotions::addEmotion("Shutdown", "blank");

  return true;
}


bool Toaster::loadNEC(const YamlNodeArray& yaml) {
  auto nec = yaml.findKeys("hardware:ir_remote:nec");
  if (nec == nullptr || nec->isObject() == false || nec->asObjects().empty()) {
    return false;
  }

  for (const auto& it : nec->asObjects()) {
    if (it.isObject() == false) {
      continue;
    }

    auto& nec_item = it.asObjects();
    uint32_t code = parse_hex(nec_item.getString("code", "0x0").c_str());
    std::string emotion = nec_item.getString("emotion");
    std::string keypress = nec_item.getString("keypress");
    if (emotion.empty() == false) {
      _ir_remote.addNEC_Emotion(code, emotion.c_str());
    }
    else if (keypress.empty() == false) {
      _ir_remote.addNEC_Keypress(code, keypress[0]);
    }
  }
  
  return true;
}


};
