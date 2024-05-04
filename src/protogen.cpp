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
const char* Toaster::_tf_version = "2024.5.1";


static const int ADC_MAX = 4095;


Toaster::Toaster() {
}


bool Toaster::begin() {
  if (_init) {
    return true;
  }

  _serialdebug.begin();

  TF_LOGI(TAG, "ToasterFirmware %s", _tf_version);

  if (!FFat.begin(false)) {
    TF_LOGE(TAG, "begin: FFat mount failed");
    return false;
  }

  YamlNodeArray yaml = YamlNodeArray::fromFile("/config.yaml", FFat);

  if (yaml.isError()) {
    TF_LOGE(TAG, "begin: config.yaml does not exist or format error (%s)", yaml.getLastError());
    return false;
  }

  const int MAX_YAML_VERSION = 2;
  _version = yaml.getInt("version", 0);
  if (_version > MAX_YAML_VERSION) {
    TF_LOGE(TAG, "begin: config.yaml version error (%d, required %d)", _version, MAX_YAML_VERSION);
    return false;
  }

  // TF_LOGD(TAG, "config.yaml\n\n%s", yaml.toString().c_str());
  
  uint32_t i2c_freq = yaml.getInt("hardware:i2c:frequency", 100000);
	if (!Wire.begin(-1, -1, i2c_freq)) {
    TF_LOGE(TAG, "begin: I2C begin failed (frequency: %d)", i2c_freq);
  }
  else {
    TF_LOGI(TAG, "begin: I2C begin succeed (frequency: %d)", i2c_freq);
  }

  if (!loadHub75(yaml)) {
    TF_LOGE(TAG, "begin: Display begin failed");
    return false;
  }

  if (!loadSidePanel(yaml)) {
    TF_LOGW(TAG, "begin: Side Display begin failed");
    return false;
  }

  refreshAutoBrightness(_default_brightness);

  if (!loadHUD(yaml)) {
    TF_LOGW(TAG, "begin: HUD begin failed");
  }

  if (_version == 1) {
    loadPhotoresistor(yaml);
  }
  else {
    loadLightSensor(yaml);
  }

  loadBoopSensor(yaml);
  loadThermometer(yaml);
  loadRemote(yaml);
  loadEmotions(yaml);
  loadPersonality(yaml);
  
  _interruptSemaphore = xSemaphoreCreateBinary();
  syncUnlock();

  _hub75_fps = yaml.getInt("hardware:hub75:fps", 60);
  Worker::begin(_hub75_fps);

  _init = true;

  xTaskCreatePinnedToCore([](void* param) {
    auto pthis = (Toaster*)param;
    while (1) {
      pthis->_hud.loop();
      pthis->_i2c_espnow.loop();
      pthis->_boopsensor.loop();
      pthis->_lightsensor.loop();
      pthis->_thermometer.loop();

      delay(1);
    }
    }, "hud", 8192, this, 1, nullptr, PRO_CPU_NUM);

  return true;
}


void Toaster::loop() {
  syncLock();

  _serialdebug.loop();
  _ir_remote.loop();

  syncUnlock();

  Worker::loop();
}


bool Toaster::work() {
  syncLock();

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

  if (_lightsensor_use) {
    float light = _lightsensor.getValue();
    if (light <= _ls_auto.getDataPoints().front().data_from
      || light >= _ls_auto.getDataPoints().back().data_from
      || abs(_ls_recent - light) >= _ls_hys) {
      _ls_recent = light;
      refreshAutoBrightness();
    }
  }
  
  syncUnlock();

  return true;
}


bool Toaster::workPerSecond() {
  char sz[256] = {0, };
  char* p = sz;

  p += sprintf(p, "FPS: %d", getRecentFPS());

  if (_hud_use) {
    p += sprintf(p, ", HUD: %d", _hud.getRecentFPS());
  }

  if (_boopsensor_use) {
    p += sprintf(p, ", Boop: %d (%d)", _boopsensor.getRecentFPS(), _boopsensor.getErrorTotal());
  }

  if (_lightsensor_use) {
    p += sprintf(p, ", LS: %d (%.1f -> %.2f)", _lightsensor.getRecentFPS(), _lightsensor.getValue(), _display.getBrightness() / 255.0f);
  }
  else {
    p += sprintf(p, ", B: %.2f", _display.getBrightness() / 255.0f);
  }

  p += sprintf(p, ", heap: %d", ESP.getFreeHeap());

  TF_LOGD(TAG, sz);

  return true;
}


bool Toaster::setEmotion(const char* emotion) {
  for (const auto& it : _emotions) {
    if (strcasecmp(it.name.c_str(), emotion) == 0) {
      setEffect(it.mouth.c_str(), it.nose.c_str(), it.eyes.c_str());
      setSideEffect(it.side.c_str());
      return true;
    }
  }

  TF_LOGW(TAG, "setEmotion: %s not found", emotion);
  
  return false;
}


void Toaster::setEffect(const char* new_effect1, const char* new_effect2, const char* new_effect3) {
  const char* new_effects[] = {new_effect1, new_effect2, new_effect3};
  uint32_t fps = 0;

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
    else {
      if (_effect[i] != nullptr) {
        _effect[i]->restart();
      }
    }
    
    if (_effect[i] != nullptr && _effect[i]->isVideoMode()) {
      if (fps != 0) {
        TF_LOGW(TAG, "setEffect: More than 1 video was found. It may not be displayed correctly.");
      }

      fps = _effect[i]->getVideoFPS();
    }
  }

  setFPS((fps > 0) ? fps : _hub75_fps);
  _hud.setFPS((fps > 0 && fps < _hud_fps) ? fps : _hud_fps);
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


void Toaster::refreshAutoBrightness() {
  float new_brightness;

  if (!_ls_auto.getValue(_ls_recent, new_brightness, true)) {
    return;
  }

  refreshAutoBrightness(new_brightness);
}


void Toaster::refreshAutoBrightness(float brightness) {
  uint8_t new_brightness_8 = (uint8_t)(brightness * 255.0f);
  _display.setBrightness(new_brightness_8);

  if (_side_display_use) {
    float new_brightness_side = std::min(brightness * _side_brightness_rate, 1.0f);
    uint8_t new_brightness_side_8 = (uint8_t)(new_brightness_side * 255.0f);
    _side_display.setBrightness(new_brightness_side_8);
  }
}


void Toaster::toast() {
  TF_LOGW(TAG, "toast: Sorry, this feature is not supported.");
}


void Toaster::syncLock(TickType_t xBlockTime) {
  uint8_t current_core = xPortGetCoreID();

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
  uint8_t current_core = xPortGetCoreID();

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


bool Toaster::loadHub75(const YamlNodeArray& yaml) {
  HUB75_I2S_CFG::i2s_pins hub75_pins;

  hub75_pins.r1 = yaml.getInt("hardware:hub75:r1_pin", 25);
  hub75_pins.g1 = yaml.getInt("hardware:hub75:g1_pin", 26);
  hub75_pins.b1 = yaml.getInt("hardware:hub75:b1_pin", 27);
  hub75_pins.r2 = yaml.getInt("hardware:hub75:r2_pin", 14);
  hub75_pins.g2 = yaml.getInt("hardware:hub75:g2_pin", 12);
  hub75_pins.b2 = yaml.getInt("hardware:hub75:b2_pin", 13);

  hub75_pins.a = yaml.getInt("hardware:hub75:a_pin", 23);
  hub75_pins.b = yaml.getInt("hardware:hub75:b_pin", 19);
  hub75_pins.c = yaml.getInt("hardware:hub75:c_pin", 5);
  hub75_pins.d = yaml.getInt("hardware:hub75:d_pin", 33);
  hub75_pins.e = yaml.getInt("hardware:hub75:e_pin", 32);
  
  hub75_pins.lat = yaml.getInt("hardware:hub75:lat_pin", 4);
  hub75_pins.oe = yaml.getInt("hardware:hub75:oe_pin", 15);
  hub75_pins.clk = yaml.getInt("hardware:hub75:clk_pin", 18);

  uint8_t min_refresh_rate = yaml.getInt("hardware:hub75:min_refresh_rate", 240);

  if (!_display.begin(HUB75_PANEL_RES_X, HUB75_PANEL_RES_Y, HUB75_PANEL_CHAIN, hub75_pins, min_refresh_rate)) {
    return false;
  }

  _default_brightness = yaml.getFloat("hardware:hub75:default_brightness", 1.0);

  return true;
}


bool Toaster::loadSidePanel(const YamlNodeArray& yaml) {
  _side_display_use = yaml.isKeysExist("hardware:side_panel");
  if (_side_display_use) {
    int side_panel_num_leds = yaml.getInt("hardware:side_panel:num_leds", 1);
    int side_panel_pin = yaml.getInt("hardware:side_panel:pin", 17);
    if (!_side_display.begin(side_panel_num_leds, side_panel_pin)) {
      return false;
    }
  }

  _side_brightness_rate = yaml.getFloat("hardware:side_panel:brightness_rate", 1.0);

  return true;
}


bool Toaster::loadHUD(const YamlNodeArray& yaml) {
  _hud_use = yaml.isKeysExist("hardware:hud");
  if (_hud_use) {
    uint8_t hud_i2c = parse_hex(yaml.getString("hardware:hud:i2c", "0x3C").c_str());
    _hud_fps = yaml.getInt("hardware:hud:fps", 30);
    if (!_hud.begin(hud_i2c, _hud_fps)) {
      return false;
    }
  }

  return true;
}


bool Toaster::loadPhotoresistor(const YamlNodeArray& yaml) {
  _lightsensor_use = yaml.isKeysExist("hardware:photoresistor");
  if (_lightsensor_use) {
    int pr_pin = yaml.getInt("hardware:photoresistor:pin", 34);
    float pr_alpha = yaml.getFloat("hardware:photoresistor:alpha", 0.98);
    float pr_alpha_init = yaml.getFloat("hardware:photoresistor:alpha_init", 0.9);
    float hys = yaml.getFloat("hardware:photoresistor:hysteresis", 0.05);
    _ls_hys = (hys * ADC_MAX);

    _lightsensor.beginLDR(pr_pin, pr_alpha, pr_alpha_init);
    _ls_recent = _lightsensor.getValue();
  
    auto cal_points = yaml.findKeys("hardware:photoresistor:calibration:points");
    if (cal_points == nullptr || cal_points->asObjects().size() < 2) {
      _ls_auto.pushDataPoint({1024, 0.25f});
      _ls_auto.pushDataPoint({3200, 1.0f});
    }
    else {
      for (const auto& it : cal_points->asObjects()) {
        if (it.isString() == false) continue;

        std::string left;
        std::string right;
        parse_separate(it.asString().c_str(), " -> ", left, right);
        float sensor = atof(left.c_str());
        float brightness = atof(right.c_str());
        _ls_auto.pushDataPoint({sensor * ADC_MAX, brightness});
      }
    }

    refreshAutoBrightness();
  }

  return true;
}


bool Toaster::loadLightSensor(const YamlNodeArray& yaml) {
  _lightsensor_use = yaml.isKeysExist("hardware:lightsensor");
  if (_lightsensor_use) {
    float ls_alpha = yaml.getFloat("hardware:lightsensor:alpha", 0.98);
    float ls_alpha_init = yaml.getFloat("hardware:lightsensor:alpha_init", 0.9);
    
    std::string ls_type = yaml.getString("hardware:lightsensor:type", "none");
    if (ls_type == "ldr") {
      int ls_pin = yaml.getInt("hardware:lightsensor:pin", 34);
      _lightsensor.beginLDR(ls_pin, ls_alpha, ls_alpha_init);

      float hys = yaml.getFloat("hardware:lightsensor:hysteresis", 0.05);
      _ls_hys = (int)(hys * ADC_MAX);
    }
    else if (ls_type == "bh1750") {
      int ls_i2c = parse_hex(yaml.getString("hardware:lightsensor:i2c", "0x23").c_str());
      _lightsensor.beginBH1750(ls_i2c, ls_alpha, ls_alpha_init);
      
      float hys = yaml.getFloat("hardware:lightsensor:hysteresis", 10);
      _ls_hys = hys;
    }
    else {
      _lightsensor_use = false;
      
      return true;
    }
    
    _ls_recent = _lightsensor.getValue();

    auto cal_points = yaml.findKeys("hardware:lightsensor:calibration:points");
    if (ls_type == "bh1750") {
      if (cal_points == nullptr || cal_points->asObjects().size() < 2) {
        _ls_auto.pushDataPoint({10, 0.5f});
        _ls_auto.pushDataPoint({50, 1.0f});
      }
      else {
        for (const auto& it : cal_points->asObjects()) {
          if (it.isString() == false) continue;

          std::string left;
          std::string right;
          parse_separate(it.asString().c_str(), " -> ", left, right);
          float sensor = atof(left.c_str());
          float brightness = atof(right.c_str());
          _ls_auto.pushDataPoint({sensor, brightness});
        }
      }
    }
    else {
      if (cal_points == nullptr || cal_points->asObjects().size() < 2) {
        _ls_auto.pushDataPoint({1024, 0.25f});
        _ls_auto.pushDataPoint({3200, 1.0f});
      }
      else {
        for (const auto& it : cal_points->asObjects()) {
          if (it.isString() == false) continue;

          std::string left;
          std::string right;
          parse_separate(it.asString().c_str(), " -> ", left, right);
          float sensor = atof(left.c_str());
          float brightness = atof(right.c_str());
          _ls_auto.pushDataPoint({sensor * ADC_MAX, brightness});
        }
      }
    }

    refreshAutoBrightness();
  }

  return true;
}


bool Toaster::loadBoopSensor(const YamlNodeArray& yaml) {
  _boopsensor_use = yaml.isKeysExist("hardware:boopsensor");
  if (_boopsensor_use) {
    std::string type = yaml.getString("hardware:boopsensor:type", "vl53l1x");
    if (type != "vl53l1x") {
      return false;
    }

    uint8_t i2c = parse_hex(yaml.getString("hardware:boopsensor:i2c", "0x29").c_str());
    uint16_t threshold = yaml.getInt("hardware:boopsensor:threshold", 20);
    int sampling = yaml.getInt("hardware:boopsensor:sampling", 3);
    
    if (_boopsensor.begin(i2c, threshold, sampling)) {
      TF_LOGI(TAG, "loadBoopSensor: VL53L1X(0x%02x) connected.", i2c);
    }
    else {
      TF_LOGW(TAG, "loadBoopSensor: VL53L1X(0x%02x) not found.", i2c);
      return false;
    }
  }

  return true;
}


bool Toaster::loadThermometer(const YamlNodeArray& yaml) {
  _thermometer_use = yaml.isKeysExist("hardware:thermometer");
  if (_thermometer_use) {
    std::string type = yaml.getString("hardware:thermometer:type", "sht31");
    if (type != "sht31") {
      return false;
    }

    uint8_t i2c = parse_hex(yaml.getString("hardware:thermometer:i2c", "0x44").c_str());
    uint32_t period = parse_time_str(yaml.getString("hardware:thermometer:period", "15s").c_str());
    
    if (_thermometer.beginSHT31(period, i2c)) {
      TF_LOGI(TAG, "loadThermometer: SHT31(0x%02x) connected.", i2c);
    }
    else {
      TF_LOGW(TAG, "loadThermometer: SHT31(0x%02x) not found.", i2c);
      return false;
    }
  }

  return true;
}


bool Toaster::loadRemote(const YamlNodeArray& yaml) {
  _espnow_use = yaml.isKeysExist("hardware:i2c_esp_now");
  if (_espnow_use) {
    uint8_t i2c = parse_hex(yaml.getString("hardware:i2c_esp_now:i2c", "0x15").c_str());
    if (_i2c_espnow.begin(i2c)) {
      TF_LOGI(TAG, "loadRemote: ESP-Now Receiver (0x%02x) connected.", i2c);
    }
    else {
      TF_LOGW(TAG, "loadRemote: ESP-Now Receiver (0x%02x) not found.", i2c);
    }
  }

  _ir_remote_use = yaml.isKeysExist("hardware:ir_remote");
  if (_ir_remote_use) {
    int ir_remote_pin = yaml.getInt("hardware:ir_remote:pin", 35);
    bool ir_remote_debug = parse_bool(yaml.getString("hardware:ir_remote:debug", "false").c_str());

    if (!_ir_remote.begin(ir_remote_pin, ir_remote_debug)) {
      TF_LOGW(TAG, "loadRemote: IR Receiver not connected. disabled.");
      return false;
    }

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
  
  if (_version == 1) {
    std::string eyes_default = yaml.getString("my_protogen:eyes_default", "eyes_normal");
    std::string nose_default = yaml.getString("my_protogen:nose_default", "nose_default");
    std::string mouth_default = yaml.getString("my_protogen:mouth_default", "mouth_default");
    std::string side_default = yaml.getString("my_protogen:side_default", "side_default");

    setEffect(mouth_default.c_str(), nose_default.c_str(), eyes_default.c_str());
    setSideEffect(side_default.c_str());
  }
  else {
    std::string default_emotion = yaml.getString("my_protogen:default_emotion", "normal");
    if (!setEmotion(default_emotion.c_str())) {
      TF_LOGE(TAG, "loadPersonality: default_emotion %s not found", default_emotion.c_str());
    }
  }

  uint32_t rainbow_single_speed = parse_time_str(yaml.getString("my_protogen:rainbow_single_speed", "20ms").c_str());
  Effect::setRainbowSingleSpeed(rainbow_single_speed);

  uint32_t rainbow_speed = parse_time_str(yaml.getString("my_protogen:rainbow_speed", "10ms").c_str());
  uint32_t rainbow_transition_pixels = atoi(yaml.getString("my_protogen:rainbow_transition_pixels", "4").c_str());
  Effect::setRainbowSpeed(rainbow_speed, rainbow_transition_pixels);

  auto g_points = yaml.findKeys("my_protogen:gradation_points");
  if (g_points == nullptr || g_points->asObjects().size() < 2) {
    Effect::setGradationPoint(0.0f, 0, 0, 255);
    Effect::setGradationPoint(1.0f, 0, 255, 0);
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

  _festive_face = parse_bool(yaml.getString("my_protogen:festive_face", "true").c_str());

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
  _emotions.push_back({"festive", "", "", "festive", "side_rainbow"});
  
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
