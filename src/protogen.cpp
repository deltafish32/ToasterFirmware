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
const char* Toaster::_tf_version = "2025.3.1";
const char* Toaster::DEFAULT_BASE_PATH = "/emotions/default";


static const int ADC_MAX = 4095;

static const char* SCRIPT_DEFAULT_PATH = "/emotions/default/script";

static const char* EMOTIONS_DIR = "/emotions";
static const char* EMOTIONS_DEFAULT_YAML = "/emotions/default/emotion.yaml";
static const char* EMOTION_DEFAULT = "default";

static const char* EMOTIONS_SHORTCUT_YAML = "/emotions/shortcut.yaml"; // OBSOLETE
static const char* SHORTCUT_DIR = "/shortcut";
static const char* SHORTCUT_DEFAULT = "default";

static const std::map<std::string, uint8_t> SIDE_PANEL_TYPE = {
  {"strip", SIDE_PANEL_STRIP},
  {"circle", SIDE_PANEL_CIRCLE},
};

static const std::map<std::string, uint8_t> RTC_FORMAT = {
  {"hm", RTC_FORMAT_HM},
  {"hms", RTC_FORMAT_HMS},
  {"hm_no_blinking", RTC_FORMAT_HM_NO_BLINKING},
  {"hms_no_blinking", RTC_FORMAT_HMS_NO_BLINKING},
};

const std::vector<std::pair<float, float>> SIDE_PANEL_CIRCLE_DATA = {
  //    angle, distance
    {   0.00f, 0.00f, },
    {   0.00f, 0.38f, },
    {  45.00f, 0.38f, },
    {  90.00f, 0.38f, },
    { 135.00f, 0.38f, },
    { 180.00f, 0.38f, },
    { 225.00f, 0.38f, },
    { 270.00f, 0.38f, },
    { 315.00f, 0.38f, },
    {   0.00f, 0.70f, },
    {  30.00f, 0.70f, },
    {  60.00f, 0.70f, },
    {  90.00f, 0.70f, },
    { 120.00f, 0.70f, },
    { 150.00f, 0.70f, },
    { 180.00f, 0.70f, },
    { 210.00f, 0.70f, },
    { 240.00f, 0.70f, },
    { 270.00f, 0.70f, },
    { 300.00f, 0.70f, },
    { 330.00f, 0.70f, },
    {   0.00f, 1.00f, },
    {  22.50f, 1.00f, },
    {  45.00f, 1.00f, },
    {  67.50f, 1.00f, },
    {  90.00f, 1.00f, },
    { 112.50f, 1.00f, },
    { 135.00f, 1.00f, },
    { 157.50f, 1.00f, },
    { 180.00f, 1.00f, },
    { 202.50f, 1.00f, },
    { 225.00f, 1.00f, },
    { 247.50f, 1.00f, },
    { 270.00f, 1.00f, },
    { 292.50f, 1.00f, },
    { 315.00f, 1.00f, },
    { 337.50f, 1.00f, },
};


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

  YamlNodeArray config_yaml = YamlNodeArray::fromFile("/config.yaml", FFat);

  if (config_yaml.isError()) {
    TF_LOGE(TAG, "begin: config.yaml does not exist or format error (%s)", config_yaml.getLastError());
    return false;
  }

  const int MAX_YAML_VERSION = 2;
  _version = config_yaml.getInt("version", 0);
  if (_version > MAX_YAML_VERSION) {
    TF_LOGE(TAG, "begin: config.yaml version error (%d, required %d)", _version, MAX_YAML_VERSION);
    return false;
  }

  // TF_LOGD(TAG, "config.yaml\n\n%s", config_yaml.toString().c_str());
  
  YamlNodeArray emotion_yaml = YamlNodeArray::fromFile(EMOTIONS_DEFAULT_YAML, FFat);

  if (emotion_yaml.isError()) {
    TF_LOGD(TAG, "begin: %s does not exist or format error (%s). try load in config.yaml", EMOTIONS_DEFAULT_YAML, emotion_yaml.getLastError());
  }
  else {
    int emotion_version = config_yaml.getInt("version", 0);
    if (emotion_version > MAX_YAML_VERSION) {
      TF_LOGE(TAG, "begin: emotions.yaml version error (%d, required %d)", emotion_version, MAX_YAML_VERSION);
      return false;
    }
  }

  uint32_t i2c_freq = config_yaml.getInt("hardware:i2c:frequency", 100000);
	if (!Wire.begin(-1, -1, i2c_freq)) {
    TF_LOGE(TAG, "begin: I2C begin failed (frequency: %d)", i2c_freq);
  }
  else {
    TF_LOGI(TAG, "begin: I2C begin succeed (frequency: %d)", i2c_freq);
  }

  if (!loadHub75(config_yaml)) {
    TF_LOGE(TAG, "begin: Display begin failed");
    return false;
  }

  if (!loadSidePanel(config_yaml)) {
    TF_LOGW(TAG, "begin: Side Display begin failed");
  }

  refreshAutoBrightness(_default_brightness);

  if (!loadHUD(config_yaml)) {
    TF_LOGW(TAG, "begin: HUD begin failed");
  }

  if (_version == 1) {
    loadPhotoresistor(config_yaml);
  }
  else {
    loadLightSensor(config_yaml);
  }

  loadBoopSensor(config_yaml);
  loadThermometer(config_yaml);
  loadRTC(config_yaml);
  loadRemote(config_yaml);
  loadPersonality(config_yaml);
  loadEmotions(emotion_yaml.isError() ? config_yaml : emotion_yaml);

  loadShortcutList();
  loadShortcut(SHORTCUT_DEFAULT);
  
  _interruptSemaphore = xSemaphoreCreateBinary();
  syncUnlock();

  _hub75_fps = config_yaml.getInt("hardware:hub75:fps", 60);
  Worker::begin(_hub75_fps);

  _init = true;

  auto core_result = xTaskCreatePinnedToCore([](void* param) {
    auto pthis = (Toaster*)param;
    while (1) {
      pthis->_hud.loop();
      pthis->_i2c_espnow.loop();
      pthis->_boopsensor.loop();
      pthis->_lightsensor.loop();
      pthis->_thermometer.loop();
      pthis->_rtc.loop();

      delay(1);
    }
    }, "hud", 8192, this, 1, nullptr, PRO_CPU_NUM);
  if (core_result != pdPASS) {
    TF_LOGE(TAG, "xTaskCreatePinnedToCore failed (%d).", core_result);
  }
  
  loadDefaultEmotion(config_yaml);

  return true;
}


void Toaster::loop() {
  _serialdebug.loop();
  _ir_remote.loop();

  Worker::loop();
}


bool Toaster::work() {
  syncLock();

  _display.beginDraw();
  _display.clear();

  timer_pf_t pf = {PF_NONE, };

  Effect::setDirty();
  for (int i = 0; i < EFFECT_MAX; i++) {
    if (_effect[i] != nullptr) {
      _effect[i]->process(_display);
      if (pf.type == PF_NONE && _effect[i]->isVideoMode()) {
        pf = _effect[i]->getVideoPF();
      }
    }
  }

  if (isAdaptiveFps() && pf.type != PF_NONE) {
    setPF(pf);
    _hud.setPF(pf);
  }
  else {
    setFPS(_hub75_fps);
    _hud.setFPS(_hud_fps);
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
  if (_serialdebug.isTextMode()) {
    return false;
  }

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
  for (size_t i = 0; i < _emotions.size(); i++) {
    if (strcasecmp(_emotions[i].name.c_str(), emotion) == 0) {
      return setEmotion(i);
    }
  }

  TF_LOGW(TAG, "setEmotion: %s not found", emotion);
  
  return false;
}


bool Toaster::setEmotion(size_t index) {
  if (index >= _emotions.size()) {
    TF_LOGW(TAG, "setEmotion: out of index (%d / %d)", index, _emotions.size());
    return false;
  }

  const auto& emotion = _emotions[index];
  setEffect(0, emotion.mouth.c_str(), emotion.base_path.c_str());
  setEffect(1, emotion.nose.c_str(), emotion.base_path.c_str());
  setEffect(2, emotion.eyes.c_str(), emotion.base_path.c_str());
  setEffect(3, emotion.special.c_str(), emotion.base_path.c_str());
  setEffect(4, emotion.special2.c_str(), emotion.base_path.c_str());
  setSideEffect(emotion.side.c_str(), emotion.base_path.c_str());

  for (int i = 0; i < EFFECT_MAX; i++) {
    if (_effect[i] != nullptr) {
      _effect[i]->restart();
    }
  }

  _emotion_index = index;

  return true;
}


const char* Toaster::getEmotionGroup() const {
  const char* p = strrchr(_emotions[_emotion_index].base_path.c_str(), '/');
  return (p != nullptr) ? (p + 1) : "";
}


void Toaster::shuffleEmotion() {
  if (_shuffle_deck.empty()) {
    TF_LOGD(TAG, "shuffle list...");

    size_t emotion_index = getEmotionIndex();
    size_t emotion_count = getEmotionCount();
    
    std::vector<int> deck;
    deck.reserve(emotion_count);
    _shuffle_deck.reserve(emotion_count);

    for (size_t i = 0; i < emotion_index; i++) {
      if (!_emotions[i].shuffle_exclude) {
        deck.push_back(i);
      }
    }
    
    for (size_t i = emotion_index + 1; i < emotion_count; i++) {
      if (!_emotions[i].shuffle_exclude) {
        deck.push_back(i);
      }
    }
    
    while (!deck.empty()) {
      int rnd = Random::random(deck.size());
      _shuffle_deck.push_back(deck[rnd]);
      deck.erase(deck.begin() + rnd);
    }
  }
  
  setEmotion(_shuffle_deck.front());
  _shuffle_deck.erase(_shuffle_deck.begin());
}


bool Toaster::setNextEmotion() {
  auto isExclude = [](const char* name) -> bool {
    return (strcasecmp(name, "white") == 0);
  };

  for (size_t i = _emotion_index + 1; i < _emotions.size(); i++) {
    if (!isExclude(_emotions[i].name.c_str())) {
      return setEmotion(i);
    }
  }

  for (size_t i = 0; i < _emotion_index; i++) {
    if (!isExclude(_emotions[i].name.c_str())) {
      return setEmotion(i);
    }
  }
  
  return false;
}

void Toaster::displayEmotionList() {
  TF_LOGI(TAG, "list of emotions:");
  for (const auto& it : _emotions) {
    if (!it.base_path.empty() && it.base_path != DEFAULT_BASE_PATH) {
      const char* p = strrchr(it.base_path.c_str(), '/');
      if (p != nullptr) {
        Serial.printf("[%s] ", p + 1);
      }
    }
    Serial.printf("%s\n", it.name.c_str());
  }
}


bool Toaster::loadShortcut(const char* shortcut) {
  for (size_t i = 0; i < _shortcut_list.size(); i++) {
    if (_shortcut_list[i] == shortcut) {
      _shortcut_index = i;
    }
  }

  YamlNodeArray yaml = YamlNodeArray::fromFile((std::string(SHORTCUT_DIR) + "/" + shortcut + ".yaml").c_str(), FFat);
  if (yaml.isError() && strcasecmp(shortcut, SHORTCUT_DEFAULT) == 0) {
    yaml = YamlNodeArray::fromFile(EMOTIONS_SHORTCUT_YAML, FFat);
  }

  if (yaml.isError()) {
    TF_LOGW(TAG, "loadShortcut: %s not found");
    return false;
  }

  _shortcuts[0][0][0] = yaml.getString("shortcut:left:index");
  _shortcuts[0][0][1] = yaml.getString("shortcut:left:index_2");
  _shortcuts[0][0][2] = yaml.getString("shortcut:left:index_3");
  _shortcuts[0][0][3] = yaml.getString("shortcut:left:index_long");
  _shortcuts[0][1][0] = yaml.getString("shortcut:left:middle");
  _shortcuts[0][1][1] = yaml.getString("shortcut:left:middle_2");
  _shortcuts[0][1][2] = yaml.getString("shortcut:left:middle_3");
  _shortcuts[0][1][3] = yaml.getString("shortcut:left:middle_long");
  _shortcuts[0][2][0] = yaml.getString("shortcut:left:ring");
  _shortcuts[0][2][1] = yaml.getString("shortcut:left:ring_2");
  _shortcuts[0][2][2] = yaml.getString("shortcut:left:ring_3");
  _shortcuts[0][2][3] = yaml.getString("shortcut:left:ring_long");
  
  _shortcuts[1][0][0] = yaml.getString("shortcut:right:index");
  _shortcuts[1][0][1] = yaml.getString("shortcut:right:index_2");
  _shortcuts[1][0][2] = yaml.getString("shortcut:right:index_3");
  _shortcuts[1][0][3] = yaml.getString("shortcut:right:index_long");
  _shortcuts[1][1][0] = yaml.getString("shortcut:right:middle");
  _shortcuts[1][1][1] = yaml.getString("shortcut:right:middle_2");
  _shortcuts[1][1][2] = yaml.getString("shortcut:right:middle_3");
  _shortcuts[1][1][3] = yaml.getString("shortcut:right:middle_long");
  _shortcuts[1][2][0] = yaml.getString("shortcut:right:ring");
  _shortcuts[1][2][1] = yaml.getString("shortcut:right:ring_2");
  _shortcuts[1][2][2] = yaml.getString("shortcut:right:ring_3");
  _shortcuts[1][2][3] = yaml.getString("shortcut:right:ring_long");

  return true;
}


bool Toaster::setEmotionShortcut(uint8_t hand, uint8_t finger, uint8_t action) {
  if (_shortcuts[hand][finger][action].empty()) {
    if (action == SHORTCUT_CLICK_COUNT_MAX) {
      return setEmotionShortcut(hand, finger, 0);
    }
    else if (action > 0) {
      return setEmotionShortcut(hand, finger, action - 1);
    }
  }

  return setEmotion(_shortcuts[hand][finger][action].c_str());
}


void Toaster::setEffect(int index, const char* new_effect, const char* base_path) {
  const char* prev_effect = (_effect[index] != nullptr) ? _effect[index]->getName() : "";

  if (strcasecmp(prev_effect, new_effect) != 0) {
    if (_effect[index] != nullptr) {
      _effect[index]->release(_display);
    }

    _effect[index] = Effect::find(new_effect, index, base_path);

    if (_effect[index] != nullptr) {
      _effect[index]->init(_display);
    }
  }
}


void Toaster::setSideEffect(const char* new_effect1, const char* base_path) {
  if (_side_display_use == false) {
    return;
  }

  const char* prev_effect = (_side_effect != nullptr) ? _side_effect->getName() : "";
  const char* new_effect = (new_effect1 != nullptr) ? new_effect1 : "";

  if (strcasecmp(prev_effect, new_effect) != 0) {
    if (_side_effect != nullptr) {
      _side_effect->release(_side_display);
    }

    _side_effect = Effect::find(new_effect, EFFECT_MAX, base_path);

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
  if (_interruptSemaphore != nullptr) {
    xSemaphoreTake(_interruptSemaphore, xBlockTime);
  }
}


void Toaster::syncUnlock() {
  if (_interruptSemaphore != nullptr) {
    xSemaphoreGive(_interruptSemaphore);
  }
}


bool Toaster::isBlank() const {
  for (int i = 0; i < EFFECT_MAX; i++) {
    if (_effect[i] != nullptr && !_effect[i]->isBlank()) {
      return false;
    }
  }

  return true;
}


bool Toaster::isAdaptiveFps() const {
  uint8_t count = 0;

  for (int i = 0; i < 5; i++) {
    if (_effect[i] != nullptr) {
      ++count;
    }
  }

  return (count <= 1);
}


bool Toaster::isGame() const {
  for (int i = 0; i < EFFECT_MAX; i++) {
    if (_effect[i] != nullptr) {
      if (_effect[i]->isGame()) {
        return true;
      }
    }
  }
  
  return false;
}


const char* Toaster::getTimeStr() const {
  static char time_str[16];

  switch (_rtc_format) {
  case RTC_FORMAT_HM:
    sprintf(time_str, "%02d%c%02d", _rtc.getHour(), _rtc.getBlink() == 0 ? ':' : ' ', _rtc.getMinute());
    break;
  case RTC_FORMAT_HMS: 
    sprintf(time_str, "%02d%c%02d%c%02d", _rtc.getHour(), _rtc.getBlink() == 0 ? ':' : ' ', _rtc.getMinute(), _rtc.getBlink() == 0 ? ':' : ' ', _rtc.getSecond());
    break;
  case RTC_FORMAT_HM_NO_BLINKING:
    sprintf(time_str, "%02d:%02d", _rtc.getHour(), _rtc.getMinute());
    break;
  case RTC_FORMAT_HMS_NO_BLINKING: 
    sprintf(time_str, "%02d:%02d:%02d", _rtc.getHour(), _rtc.getMinute(), _rtc.getSecond());
    break;
  }
  
  return time_str;
}


bool Toaster::isEmotionExist(const char* name) const {
  for (int i = 0; i < _emotions.size(); i++) {
    if (_emotions[i].name == name) {
      return true;
    }
  }

  return false;
}


void Toaster::addHUDEmotion(const char* base_path, const char* emotion, const char* display_name) {
  const char* group;

  if (strcmp(base_path, DEFAULT_BASE_PATH) == 0) {
    group = EMOTION_DEFAULT;
  }
  else {
    const char* p = strrchr(base_path, '/');
    if (p == nullptr) {
      return;
    }
    
    group = p + 1;
  }

  if (display_name == nullptr) {
    display_name = emotion;
  }

  for (auto& it : hud_emotions) {
    if (strcasecmp(it.getGroupName(), group) == 0) {
      it.addEmotion(display_name, emotion);
      return;
    }
  }

  hud_emotions.push_back(HUDEmotions(group));
  hud_emotions.back().addEmotion(display_name, emotion);
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
    auto type = SIDE_PANEL_TYPE.find(yaml.getString("hardware:side_panel:type", "strip"));
    _side_panel_type = (type != SIDE_PANEL_TYPE.end()) ? type->second : SIDE_PANEL_STRIP;

    int side_panel_num_leds = yaml.getInt("hardware:side_panel:num_leds", 1);
    int side_panel_pin = yaml.getInt("hardware:side_panel:pin", 17);
    bool side_panel_reversed = parse_bool(yaml.getString("hardware:side_panel:reversed", "false").c_str());

    if (!_side_display.begin(side_panel_num_leds, side_panel_pin, side_panel_reversed)) {
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
      TF_LOGW(TAG, "loadHUD: SSD1306(0x%02x) not found", hud_i2c);
      return false;
    }

    bool hud_dithering = parse_bool(yaml.getString("hardware:hud:dithering", "true").c_str());
    _hud.setDithering(hud_dithering);
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


bool Toaster::loadRTC(const YamlNodeArray& yaml) {
  _rtc_use = yaml.isKeysExist("hardware:rtc");
  if (_rtc_use) {
    std::string type = yaml.getString("hardware:rtc:type", "ds3231");
    if (type != "ds3231") {
      return false;
    }

    bool nofail = parse_bool(yaml.getString("hardware:rtc:nofail", "true").c_str());

    uint8_t i2c = 0x68;
    if (_rtc.beginDS3231(nofail)) {
      if (_rtc.isSynced()) {
        TF_LOGI(TAG, "loadRTC: DS3231(0x%02x) connected.", i2c);
      }
      else {
        TF_LOGI(TAG, "loadRTC: DS3231(0x%02x) not found, use as uptime.", i2c);
      }
    }
    else {
      TF_LOGW(TAG, "loadRTC: DS3231(0x%02x) not found.", i2c);
      return false;
    }

    auto format = RTC_FORMAT.find(yaml.getString("hardware:rtc:format", "hm"));
    _rtc_format = (format != RTC_FORMAT.end()) ? format->second : RTC_FORMAT_HM;
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

    _i2c_espnow.clearWhitelist();

    auto whitelist = yaml.findKeys("hardware:i2c_esp_now:whitelist");
    if (whitelist != nullptr) {
      for (const auto& it : whitelist->asObjects()) {
        if (it.isString() == false) continue;
        
        uint8_t mac[6];
        if (parse_mac(it.asString().c_str(), mac)) {
          TF_LOGD(TAG, "loadRemote: whitelist added %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
          _i2c_espnow.addWhitelist(mac);
        }
      }
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

  color_map.insert({"color_eyes", yaml.getString("my_protogen:color_eyes", "#00ff00")});
  color_map.insert({"color_nose", yaml.getString("my_protogen:color_nose", "color_eyes")});
  color_map.insert({"color_mouth", yaml.getString("my_protogen:color_mouth", "color_eyes")});
  color_map.insert({"color_side", yaml.getString("my_protogen:color_side", "color_eyes")});

  color_map.insert({"color_right_eyes", yaml.getString("my_protogen:color_right_eyes", "color_eyes")});
  color_map.insert({"color_right_nose", yaml.getString("my_protogen:color_right_nose", "color_nose")});
  color_map.insert({"color_right_mouth", yaml.getString("my_protogen:color_right_mouth", "color_mouth")});
  color_map.insert({"color_right_side", yaml.getString("my_protogen:color_right_side", "color_side")});

  if (!color_map.empty()) {
    auto set_color = [&color_map](const char* name, uint8_t index, uint32_t def_color) -> bool {
      std::string color_ref = name;

      for (int i = 0; i < 8; i++) {
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

    set_color("color_eyes", Effect::PC_EYES, 0x00ff00);
    set_color("color_nose", Effect::PC_NOSE, 0x00ff00);
    set_color("color_mouth", Effect::PC_MOUTH, 0x00ff00);
    set_color("color_side", Effect::PC_SIDE, 0x00ff00);
    
    set_color("color_right_eyes", Effect::PC_EYES + Effect::PC_MAX, 0x00ff00);
    set_color("color_right_nose", Effect::PC_NOSE + Effect::PC_MAX, 0x00ff00);
    set_color("color_right_mouth", Effect::PC_MOUTH + Effect::PC_MAX, 0x00ff00);
    set_color("color_right_side", Effect::PC_SIDE + Effect::PC_MAX, 0x00ff00);
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
  
  uint32_t rainbow_single_speed = parse_time_str(yaml.getString("my_protogen:rainbow_single_speed", "20ms").c_str());
  Effect::setRainbowSingleSpeed(rainbow_single_speed);

  uint32_t rainbow_speed = parse_time_str(yaml.getString("my_protogen:rainbow_speed", "10ms").c_str());
  uint32_t rainbow_transition_pixels = atoi(yaml.getString("my_protogen:rainbow_transition_pixels", "4").c_str());
  Effect::setRainbowSpeed(rainbow_speed, rainbow_transition_pixels);

  auto g_points = yaml.findKeys("my_protogen:gradation_points");
  loadGradation(0, g_points);

  auto g_right_points = yaml.findKeys("my_protogen:gradation_right_points");
  if (g_right_points == nullptr) {
    g_right_points = g_points;
  }
  loadGradation(1, g_right_points);

  _festive_face = parse_bool(yaml.getString("my_protogen:festive_face", "true").c_str());

  return true;
}


void Toaster::loadGradation(uint8_t index, const YamlNode* node) {
  Effect::clearGradationPoint(index);

  if (node == nullptr || node->asObjects().size() < 2) {
    Effect::setGradationPoint(index, 0.0f, 0, 0, 255);
    Effect::setGradationPoint(index, 1.0f, 0, 255, 0);
  }
  else {
    for (const auto& it : node->asObjects()) {
      if (it.isString() == false) continue;

      std::string left;
      std::string right;
      parse_separate(it.asString().c_str(), " -> ", left, right);
      uint32_t color = parse_hex(right.c_str());
      Effect::setGradationPoint(index, atof(left.c_str()), (color >> 0) & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff);
    }
  }
}


bool Toaster::loadEmotions(const YamlNodeArray& yaml) {
  auto emotions = yaml.findKeys("emotions");
  if (emotions == nullptr || emotions->isObject() == false || emotions->asObjects().empty()) {
    return false;
  }

  bool is_default_loaded = false;
  
  for (const auto& it : emotions->asObjects()) {
    const auto& emotion = it.asObjects();
    
    auto name = emotion.findKeys("name");
    if (name != nullptr && name->asString() == "default") {
      auto eyes = emotion.findKeys("eyes");
      if (eyes == nullptr) continue;

      auto nose = emotion.findKeys("nose");
      if (nose == nullptr) continue;

      auto mouth = emotion.findKeys("mouth");
      if (mouth == nullptr) continue;

      auto special = emotion.findKeys("special");
      if (special == nullptr) continue;

      auto special2 = emotion.findKeys("special2");
      if (special2 == nullptr) continue;

      auto side = emotion.findKeys("side");
      if (side == nullptr) continue;

      _emotion_default.name = name->asString();
      _emotion_default.eyes = eyes->asString();
      _emotion_default.nose = nose->asString();
      _emotion_default.mouth = mouth->asString();
      _emotion_default.special = special->asString();
      _emotion_default.special2 = special2->asString();
      _emotion_default.side = side->asString();
      is_default_loaded = true;
      break;
    }
  }

  if (is_default_loaded == false) {
    return false;
  }

  _emotions.push_back({DEFAULT_BASE_PATH, "blank", "", "", "blank", "", "", "blank", true});
  _emotions.push_back({DEFAULT_BASE_PATH, "white", "", "", "white", "", "", "white", true});
  _emotions.push_back({DEFAULT_BASE_PATH, "festive", "", "", "festive", "", "", "side_rainbow", false});
  _emotions.push_back({DEFAULT_BASE_PATH, "clock", "", "", "clock", "", "", "side_default", false});
  _emotions.push_back({DEFAULT_BASE_PATH, "dino", "", "", "dino", "", "", "side_rainbow", false});

  hud_emotions.clear();
  hud_emotions.push_back(HUDEmotions(EMOTION_DEFAULT));
  
  size_t e_count = loadEmotionEachYaml(yaml, DEFAULT_BASE_PATH);
  size_t e_total_count = e_count;
  TF_LOGI(TAG, "%d emotions loaded", e_count);

  File dir = FFat.open(EMOTIONS_DIR);
  if (dir) {
    String filename = dir.getNextFileName();
  
    while (!filename.isEmpty()) {
      File sub_dir = FFat.open(filename);
      if (sub_dir) {
        if (sub_dir.isDirectory()) {
          const char* p = strrchr(filename.c_str(), '/');
          if (p == nullptr || strcasecmp(p + 1, EMOTION_DEFAULT) != 0) {
            auto e_yaml = YamlNodeArray::fromFile((filename + "/emotions.yaml").c_str(), FFat);
            e_count = loadEmotionEachYaml(e_yaml, filename.c_str());

            e_total_count += e_count;
            TF_LOGI(TAG, "%s - %d emotions loaded", filename.c_str(), e_count);
          }
        }

        sub_dir.close();
      }

      filename = dir.getNextFileName();
    }

    dir.close();
  }
  
  TF_LOGI(TAG, "total %d emotions loaded!", e_total_count);

  addHUDEmotion(DEFAULT_BASE_PATH, "festive", "~(^w^)~");
  addHUDEmotion(DEFAULT_BASE_PATH, "clock", "clock");
  addHUDEmotion(DEFAULT_BASE_PATH, "dino", "dino");
  addHUDEmotion(DEFAULT_BASE_PATH, "blank", "Shutdown");

  return true;
}


size_t Toaster::loadEmotionEachYaml(const YamlNodeArray& yaml, const char* base_path) {
  auto emotions = yaml.findKeys("emotions");
  if (emotions == nullptr || emotions->isObject() == false || emotions->asObjects().empty()) {
    return false;
  }

  size_t count = 0;

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
    auto special = emotion.findKeys("special");
    auto special2 = emotion.findKeys("special2");
    auto side = emotion.findKeys("side");

    if (name->asString() == "default") {
      continue;
    }
    
    if (isEmotionExist(name->asString().c_str())) {
      TF_LOGW(TAG, "emotion (%s) already exist. ignored.", name->asString().c_str());
      continue;
    }

    _emotions.push_back({base_path, 
      name->asString(), 
      (eyes == nullptr) ? _emotion_default.eyes : eyes->asString(), 
      (nose == nullptr) ? _emotion_default.nose : nose->asString(), 
      (mouth == nullptr) ? _emotion_default.mouth : mouth->asString(), 
      (special == nullptr) ? _emotion_default.special : special->asString(), 
      (special2 == nullptr) ? _emotion_default.special2 : special2->asString(), 
      (side == nullptr) ? _emotion_default.side : side->asString(),
      false});
    
    addHUDEmotion(base_path, name->asString().c_str());

    ++count;
  }
  
  return count;
}


bool Toaster::loadDefaultEmotion(const YamlNodeArray& yaml) {
  if (_version == 1) {
    std::string eyes_default = yaml.getString("my_protogen:eyes_default", "eyes_normal");
    std::string nose_default = yaml.getString("my_protogen:nose_default", "nose_default");
    std::string mouth_default = yaml.getString("my_protogen:mouth_default", "mouth_default");
    std::string side_default = yaml.getString("my_protogen:side_default", "side_default");

    setEffect(0, mouth_default.c_str(), SCRIPT_DEFAULT_PATH);
    setEffect(1, nose_default.c_str(), SCRIPT_DEFAULT_PATH);
    setEffect(2, eyes_default.c_str(), SCRIPT_DEFAULT_PATH);
    setEffect(3, "", SCRIPT_DEFAULT_PATH);
    setEffect(4, "", SCRIPT_DEFAULT_PATH);
    setSideEffect(side_default.c_str(), SCRIPT_DEFAULT_PATH);
  }
  else {
    std::string default_emotion = yaml.getString("my_protogen:default_emotion", "normal");
    if (!setEmotion(default_emotion.c_str())) {
      TF_LOGE(TAG, "loadPersonality: default_emotion %s not found", default_emotion.c_str());
    }
  }

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
    std::string mode = nec_item.getString("mode");
    if (emotion.empty() == false) {
      _ir_remote.addNEC_Emotion(code, emotion.c_str());
    }
    else if (keypress.empty() == false) {
      uint8_t keypress_code = (keypress.length() == 1) ? keypress[0] : parse_hex(keypress.c_str());
      _ir_remote.addNEC_Keypress(code, keypress_code, (strcasecmp(mode.c_str(), "keycode") == 0) ? 1 : 0);
    }
  }
  
  return true;
}


bool Toaster::loadShortcutList() {
  _shortcut_list.push_back(SHORTCUT_DEFAULT);

  File dir = FFat.open(SHORTCUT_DIR);
  if (dir) {
    String filename = dir.getNextFileName();
  
    while (!filename.isEmpty()) {
      const char* p1 = strrchr(filename.c_str(), '/');
      if (p1 != nullptr) {
        const char* p2 = strrchr(p1 + 1, '.');
        if (p2 != nullptr) {
          std::string shortcut_name = std::string(p1 + 1, p2 - p1 - 1);
          if (strcasecmp(shortcut_name.c_str(), SHORTCUT_DEFAULT) != 0) {
            _shortcut_list.push_back(shortcut_name);
          }
        }
      }

      filename = dir.getNextFileName();
    }

    dir.close();
  }
  
  TF_LOGI(TAG, "total %d shortcuts loaded!", _shortcut_list.size());

  return false;
}


};
