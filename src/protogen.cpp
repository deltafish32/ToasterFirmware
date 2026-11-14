#include "protogen.h"
#include "effects/effects.h"

#include "huds/hud.h"
#include "huds/hud_emotions.h"
#include "huds/hud_usb_storage.h"
#include "hal/usb_storage.h"
#include "lib/logger.h"
#include "lib/strutil.h"
#include <map>

#ifdef USE_SD
#include "sd_diskio.h"
#include "ff.h"
#include "diskio.h"
#endif


namespace toaster {

static const char* TAG = "Protogen";

static void drawUsbStorageHUD(const UsbStorage::Status& status);


#ifdef USE_SD
// Checks whether a card responds, without mounting it (e.g. exFAT or unformatted cards).
static bool probeSdCard() {
  uint8_t pdrv = sdcard_init(PIN_FSPI_CS, &SPI, 4000000);
  if (pdrv == 0xFF) {
    return false;
  }

  bool present = (ff_disk_initialize(pdrv) & STA_NOINIT) == 0;
  sdcard_uninit(pdrv);

  return present;
}
#endif


Toaster Protogen;
const char* Toaster::_tf_version = "2026.1.1";
const char* Toaster::DEFAULT_BASE_PATH = "/emotions/default";


static const int ADC_MAX = 4095;

// Reads a positive integer setting. Zero or negative values fall back to def_value.
static uint32_t get_positive_int(const YamlNodeArray& yaml, const char* keys, int def_value) {
  int value = yaml.getInt(keys, def_value);
  if (value <= 0) {
    TF_LOGW(TAG, "%s must be greater than 0 (%d), using %d", keys, value, def_value);
    return def_value;
  }
  return value;
}

// Reads a time setting such as "10ms" or "15s". A missing or unknown unit, or zero, falls back to def_value.
static uint32_t get_time_ms(const YamlNodeArray& yaml, const char* keys, const char* def_value) {
  std::string str = yaml.getString(keys, def_value);
  uint32_t value = parse_time_str(str.c_str());
  if (value == 0) {
    TF_LOGW(TAG, "%s: invalid time '%s' (units: ms, s, min, h, d), using %s", keys, str.c_str(), def_value);
    value = parse_time_str(def_value);
  }
  return value;
}

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

static const std::map<std::string, uint8_t> ESP_NOW_TYPE = {
  {"i2c", ESP_NOW_I2C},
  {"local", ESP_NOW_LOCAL},
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

static const std::map<std::string, PROTOGEN_COLOR_MODE> COLOR_MODE = {
  {"personal", PCM_PERSONAL},
  {"personal2", PCM_PERSONAL2},
  {"personal3", PCM_PERSONAL3},
  {"original", PCM_ORIGINAL},
  {"rainbow_single", PCM_RAINBOW_SINGLE},
  {"rainbow", PCM_RAINBOW},
  {"gradation", PCM_GRADATION},
};

Toaster::Toaster() {
}


bool Toaster::begin() {
  if (_init) {
    return true;
  }

  _serialdebug.begin();

  TF_LOGI(TAG, "ToasterFirmware %s", _tf_version);

  if (UsbStorage::isActive()) {
    prepareUsbStorage();

    // Does not return on success (reboots on eject/disconnect).
    UsbStorage::run(_hud_use ? drawUsbStorageHUD : nullptr);

    TF_LOGE(TAG, "begin: USB storage mode failed");
    return false;
  }

  if (psramFound()) {
    psramInit();
    TF_LOGI(TAG, "PSRAM found: %d", ESP.getPsramSize());
  }

#ifdef USE_SD
  SPI.begin();

  if (SD.begin(PIN_FSPI_CS, SPI)) {
    _sd_use = true;

    TF_LOGI(TAG, "SD card found (type: %d, size: %lld)", (int)SD.cardType(), SD.cardSize() / 1024 / 1024);
  }
  else if (probeSdCard()) {
    _sd_error = true;

    TF_LOGE(TAG, "begin: SD card found but cannot be read. Format it as FAT32.");
  }
  else {
    TF_LOGW(TAG, "begin: SD begin failed");
  }

  UsbStorage::setSdCardPresent(_sd_use || _sd_error);
#endif

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

  std::string log_level_str = config_yaml.getString("log_level");
  if (!log_level_str.empty()) {
    tf_log_level_t log_level;
    if (tf_log_level_from_string(log_level_str.c_str(), &log_level)) {
      tf_log_set_level(log_level);
    }
    else {
      TF_LOGW(TAG, "begin: invalid log_level (%s)", log_level_str.c_str());
    }
  }

  // TF_LOGD(TAG, "config.yaml\n\n%s", config_yaml.toString().c_str());
  
  YamlNodeArray emotion_yaml = YamlNodeArray::fromFile(EMOTIONS_DEFAULT_YAML, FFat);

  if (emotion_yaml.isError()) {
    TF_LOGD(TAG, "begin: %s does not exist or format error (%s). try load in config.yaml", EMOTIONS_DEFAULT_YAML, emotion_yaml.getLastError());
  }
  else {
    int emotion_version = emotion_yaml.getInt("version", 0);
    if (emotion_version > MAX_YAML_VERSION) {
      TF_LOGE(TAG, "begin: %s version error (%d, required %d)", EMOTIONS_DEFAULT_YAML, emotion_version, MAX_YAML_VERSION);
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

  _hub75_fps = get_positive_int(config_yaml, "hardware:hub75:fps", 60);
  Worker::begin(_hub75_fps);

  loadDefaultEmotion(config_yaml);

  _init = true;

  auto core_result = xTaskCreatePinnedToCore([](void* param) {
    auto pthis = (Toaster*)param;
    while (1) {
      // Entering USB storage mode: stop here so that the reboot does not interrupt an I2C transfer.
      if (pthis->_usb_storage_request) {
        if (!pthis->_hud_task_paused) {
          if (pthis->_hud_use && pthis->_hud._oled.getBuffer() != nullptr) {
            UsbStorage::Status status = {};
            status.state = UsbStorage::STATE_RESTARTING;
            hud_usb_storage.draw(pthis->_hud._oled, status);
          }
          pthis->_hud_task_paused = true;
        }
        delay(10);
        continue;
      }

      pthis->_hud.loop();
      pthis->_espnow.loop();
      pthis->_boopsensor.loop();
      pthis->_lightsensor.loop();
      pthis->_thermometer.loop();
      if (pthis->_rtc.isBegin()) {
        pthis->_rtc.loop();
      }

      delay(1);
    }
    }, "hud", 4096, this, 1, &_hud_task, PRO_CPU_NUM);
  if (core_result != pdPASS) {
    TF_LOGE(TAG, "xTaskCreatePinnedToCore failed (%d).", core_result);
  }

  return true;
}


void Toaster::loop() {
  _serialdebug.loop();
  _ir_remote.loop();

  if (_usb_storage_request) {
    enterUsbStorage();
  }

  Worker::loop();
}


bool Toaster::requestUsbStorage() {
  if (!UsbStorage::isSupported()) {
    TF_LOGW(TAG, "requestUsbStorage: USB storage is not supported in this build.");
    return false;
  }

  _usb_storage_request = true;

  return true;
}


void Toaster::enterUsbStorage() {
  // Called from the rendering task, so LEDs are not turned off mid-frame.
  _display.off();

  if (_side_display_use) {
    _side_display.off();
  }

  // Wait until the HUD task finishes its I2C transfer and stops.
  uint32_t wait_start = millis();
  while (!_hud_task_paused && millis() - wait_start < 1000) {
    delay(1);
  }

  UsbStorage::reboot();
}


// A reset in the middle of an I2C transfer can leave a slave holding SDA low; release the bus with SCL pulses and a STOP.
static void recoverI2CBus(int sda, int scl) {
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delayMicroseconds(10);

  pinMode(scl, OUTPUT_OPEN_DRAIN);
  for (int i = 0; i < 9 && digitalRead(sda) == LOW; i++) {
    digitalWrite(scl, LOW);
    delayMicroseconds(10);
    digitalWrite(scl, HIGH);
    delayMicroseconds(10);
  }

  // STOP: SDA LOW -> HIGH while SCL is HIGH
  pinMode(sda, OUTPUT_OPEN_DRAIN);
  digitalWrite(scl, LOW);
  delayMicroseconds(10);
  digitalWrite(sda, LOW);
  delayMicroseconds(10);
  digitalWrite(scl, HIGH);
  delayMicroseconds(10);
  digitalWrite(sda, HIGH);
  delayMicroseconds(10);

  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
}


void Toaster::prepareUsbStorage() {
  // If the config cannot be read (e.g. fatal config error), try the HUD at the default address.
  bool hud_use = true;
  uint8_t hud_i2c = 0x3C;
  uint32_t i2c_freq = 100000;

  if (FFat.begin(false)) {
    YamlNodeArray config_yaml = YamlNodeArray::fromFile("/config.yaml", FFat);
    FFat.end();

    if (!config_yaml.isError()) {
      // HUB75 OE is active low: hold it HIGH to blank the panel.
      int oe_pin = config_yaml.getInt("hardware:hub75:oe_pin", 15);
      pinMode(oe_pin, OUTPUT);
      digitalWrite(oe_pin, HIGH);

      // NeoPixels keep their last color across resets, so send black.
      if (config_yaml.isKeysExist("hardware:side_panel")) {
        int side_panel_num_leds = config_yaml.getInt("hardware:side_panel:num_leds", 1);
        int side_panel_pin = config_yaml.getInt("hardware:side_panel:pin", 17);

        if (_side_display.begin(side_panel_num_leds, side_panel_pin, false)) {
          _side_display.off();
        }
      }

      hud_use = config_yaml.isKeysExist("hardware:hud");
      hud_i2c = parse_hex(config_yaml.getString("hardware:hud:i2c", "0x3C").c_str());
      i2c_freq = config_yaml.getInt("hardware:i2c:frequency", 100000);
    }
  }

  // HUD: used only if it responds
  if (hud_use) {
    recoverI2CBus(SDA, SCL);

    if (Wire.begin(-1, -1, i2c_freq)) {
      Wire.beginTransmission(hud_i2c);
      if (Wire.endTransmission() == 0 && _hud._oled.begin(SSD1306_SWITCHCAPVCC, hud_i2c)) {
        _hud_use = true;
      }
    }

    TF_LOGI(TAG, "prepareUsbStorage: HUD(0x%02x) %s", hud_i2c, _hud_use ? "found" : "not found");
  }
}


static void drawUsbStorageHUD(const UsbStorage::Status& status) {
  hud_usb_storage.draw(Protogen._hud._oled, status);
}


bool Toaster::work() {
  syncLock();

  if (!_next_emotion.empty()) {
    setEmotion(_next_emotion.c_str());
    _next_emotion.clear();
  }

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

  if (_lightsensor_use && _auto_brightness) {
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
  size_t len = 0;

  appendf(sz, sizeof(sz), len, "FPS: %d", getRecentFPS());

  if (_hud_use) {
    appendf(sz, sizeof(sz), len, ", HUD: %d", _hud.getRecentFPS());
  }

  if (_boopsensor_use) {
    appendf(sz, sizeof(sz), len, ", Boop: %d (%d)", _boopsensor.getRecentFPS(), _boopsensor.getErrorTotal());
  }

  if (_lightsensor_use) {
    appendf(sz, sizeof(sz), len, ", LS: %d (%.1f -> %.2f)", _lightsensor.getRecentFPS(), _lightsensor.getValue(), _display.getBrightness() / 255.0f);
  }
  else {
    appendf(sz, sizeof(sz), len, ", B: %.2f", _display.getBrightness() / 255.0f);
  }

  appendf(sz, sizeof(sz), len, ", heap: %d", ESP.getFreeHeap());

  if (psramFound()) {
    appendf(sz, sizeof(sz), len, " (%d)", ESP.getFreePsram());
  }

  // if (_hud_task != nullptr) {
  //   appendf(sz, sizeof(sz), len, ", hud stack: %u", (unsigned)uxTaskGetStackHighWaterMark(_hud_task));
  // }

  TF_LOGD(TAG, "%s", sz);

  return true;
}


void Toaster::setNextEmotion(const char* emotion) {
  _next_emotion = emotion;
}


bool Toaster::setEmotion(const char* emotion) {
  TF_LOGD(TAG, "setEmotion: %s", emotion);
  
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


const char* Toaster::getEmotionGroup(size_t index) const {
  if (index >= _emotions.size()) {
    return "";
  }

  const char* p = strrchr(_emotions[index].base_path.c_str(), '/');
  return (p != nullptr) ? (p + 1) : "";
}


int Toaster::shuffleEmotion() {
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

  if (_shuffle_deck.empty()) {
    // no other emotion to shuffle to
    return -1;
  }

  int index = _shuffle_deck.front();
  _shuffle_deck.erase(_shuffle_deck.begin());
  setNextEmotion(_emotions[index].name.c_str());

  return index;
}


bool Toaster::setEmotionNext() {
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
  YamlNodeArray yaml = YamlNodeArray::fromFile((std::string(SHORTCUT_DIR) + "/" + shortcut + ".yaml").c_str(), FFat);
  if (yaml.isError() && strcasecmp(shortcut, SHORTCUT_DEFAULT) == 0) {
    yaml = YamlNodeArray::fromFile(EMOTIONS_SHORTCUT_YAML, FFat);
  }

  if (yaml.isError()) {
    TF_LOGW(TAG, "loadShortcut: %s not found", shortcut);
    return false;
  }

  for (size_t i = 0; i < _shortcut_list.size(); i++) {
    if (_shortcut_list[i] == shortcut) {
      _shortcut_index = i;
      break;
    }
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


const char* Toaster::getEmotionShortcut(uint8_t hand, uint8_t finger, uint8_t action) {
  if (_shortcuts[hand][finger][action].empty()) {
    if (action == SHORTCUT_CLICK_COUNT_MAX) {
      return getEmotionShortcut(hand, finger, 0);
    }
    else if (action > 0) {
      return getEmotionShortcut(hand, finger, action - 1);
    }
  }

  return _shortcuts[hand][finger][action].c_str();
}


void Toaster::setEffect(int index, const char* new_effect, const char* base_path) {
  const char* path = (base_path[0] == 0) ? DEFAULT_BASE_PATH : base_path;
  bool same = (_effect[index] != nullptr) ? _effect[index]->isSameEffect(new_effect, path) : (new_effect[0] == 0);

  if (!same) {
    if (_effect[index] != nullptr) {
      _effect[index]->release(_display);
    }

    _effect[index] = Effect::find(new_effect, index, path);

    if (_effect[index] != nullptr) {
      _effect[index]->init(_display);
    }
  }
}


void Toaster::setSideEffect(const char* new_effect1, const char* base_path) {
  if (_side_display_use == false) {
    return;
  }

  const char* new_effect = (new_effect1 != nullptr) ? new_effect1 : "";
  bool same = (_side_effect != nullptr) ? _side_effect->isSameEffect(new_effect, base_path) : (new_effect[0] == 0);

  if (!same) {
    if (_side_effect != nullptr) {
      _side_effect->release(_side_display);
    }

    _side_effect = Effect::find(new_effect, EFFECT_MAX, base_path);

    if (_side_effect != nullptr) {
      _side_effect->init(_side_display);
    }
  }
}


bool Toaster::setColorMode(const char* mode) {
  auto color_mode = COLOR_MODE.find(mode);
  if (color_mode == COLOR_MODE.end()) {
    return false;
  }

  return Effect::setColorMode(color_mode->second);
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


void Toaster::setManualBrightness(float brightness) {
  if (_lightsensor_use && _auto_brightness) {
    _auto_brightness = false;
    TF_LOGI(TAG, "auto brightness paused (manual brightness).");
  }

  refreshAutoBrightness(brightness);
}


void Toaster::setAutoBrightness(bool enable) {
  _auto_brightness = enable;
  TF_LOGI(TAG, "auto brightness %s.", enable ? "enabled" : "paused");

  if (enable && _lightsensor_use) {
    refreshAutoBrightness();
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

  for (int i = 0; i < EFFECT_MAX; i++) {
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
    snprintf(time_str, sizeof(time_str), "%02d%c%02d", _rtc.getHour(), _rtc.getBlink() == 0 ? ':' : ' ', _rtc.getMinute());
    break;
  case RTC_FORMAT_HMS: 
    snprintf(time_str, sizeof(time_str), "%02d%c%02d%c%02d", _rtc.getHour(), _rtc.getBlink() == 0 ? ':' : ' ', _rtc.getMinute(), _rtc.getBlink() == 0 ? ':' : ' ', _rtc.getSecond());
    break;
  case RTC_FORMAT_HM_NO_BLINKING:
    snprintf(time_str, sizeof(time_str), "%02d:%02d", _rtc.getHour(), _rtc.getMinute());
    break;
  case RTC_FORMAT_HMS_NO_BLINKING: 
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", _rtc.getHour(), _rtc.getMinute(), _rtc.getSecond());
    break;
  }
  
  return time_str;
}


bool Toaster::isEmotionExist(const char* name) const {
  return (findEmotion(name) >= 0);
}


int Toaster::findEmotion(const char* name) const {
  for (int i = 0; i < _emotions.size(); i++) {
    if (_emotions[i].name == name) {
      return i;
    }
  }

  return -1;
}


static std::string makeGroupName(const char* base_path, bool nosd = false) {
  std::string group;

  if (base_path[0] == 0) {
    group = EMOTION_DEFAULT;
  }
  else if (!nosd && strncmp(base_path, "/sd/", 4) == 0) {
    const char* p = strrchr(base_path + 3, '/');
    if (p == nullptr) {
      return "";
    }
    
    group = "sd/";
    group += p + 1;
  }
  else {
    const char* p = strrchr(base_path, '/');
    if (p == nullptr) {
      return "";
    }
    
    group = p + 1;
  }

  return group;
}


void Toaster::addHUDEmotion(const char* base_path, const char* emotion, const char* display_name) {
  std::string group = makeGroupName(base_path);

  if (display_name == nullptr) {
    display_name = emotion;
  }

  for (auto& it : hud_emotions) {
    if (strcasecmp(it.getGroupName(), group.c_str()) == 0) {
      it.addEmotion(display_name, emotion);
      return;
    }
  }

  hud_emotions.push_back(HUDEmotions(group.c_str()));
  hud_emotions.back().addEmotion(display_name, emotion);
}


bool Toaster::removeHUDEmotion(const char* base_path, const char* emotion) {
  // base_path is the stored path of the emotion (may be /sd/...), resolve the group like addHUDEmotion
  std::string group = makeGroupName(base_path);

  for (auto it = hud_emotions.begin(); it != hud_emotions.end(); ++it) {
    if (strcasecmp(it->getGroupName(), group.c_str()) == 0) {
      return it->removeEmotion(emotion);
    }
  }

  return false;
}


void Toaster::clearEmptyHUDEmotions() {
  for (auto it = hud_emotions.begin(); it != hud_emotions.end(); ) {
    if (it->isEmpty()) {
      it = hud_emotions.erase(it);
    }
    else {
      ++it;
    }
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

  uint16_t min_refresh_rate = yaml.getInt("hardware:hub75:min_refresh_rate", 480);

  if (!_display.begin(HUB75_PANEL_RES_X, HUB75_PANEL_RES_Y, HUB75_PANEL_CHAIN, hub75_pins, min_refresh_rate)) {
    return false;
  }

  _default_brightness = yaml.getFloat("hardware:hub75:default_brightness", 1.0);
  
  TF_LOGD(TAG, "HUB75 refresh rate: %d -> %d", (int)min_refresh_rate, _display.get_refresh_rate());

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
    _hud_fps = get_positive_int(yaml, "hardware:hud:fps", 30);
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
      if (!_lightsensor.beginBH1750(ls_i2c, ls_alpha, ls_alpha_init)) {
        _lightsensor_use = false;
        TF_LOGW(TAG, "loadLightSensor: BH1750(0x%02x) not found", ls_i2c);
        return false;
      }
      
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
    uint32_t period = get_time_ms(yaml, "hardware:thermometer:period", "15s");
    
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
  bool key_exist = yaml.isKeysExist("hardware:esp_now");
  bool key_exist2 = yaml.isKeysExist("hardware:i2c_esp_now");
  _espnow_use = key_exist || key_exist2;
  if (_espnow_use) {
    if (key_exist) {
      auto type = ESP_NOW_TYPE.find(yaml.getString("hardware:esp_now:type", "none"));
      _esp_now_type = (type != ESP_NOW_TYPE.end()) ? type->second : ESP_NOW_NONE;
    }
    else {
      // legacy key (hardware:i2c_esp_now) always means the I2C receiver
      _esp_now_type = ESP_NOW_I2C;
    }
    if (_esp_now_type == ESP_NOW_I2C) {
      uint8_t i2c = parse_hex(yaml.getString(key_exist ? "hardware:esp_now:i2c" : "hardware:i2c_esp_now:i2c", "0x15").c_str());
      if (_espnow.beginI2C(i2c)) {
        TF_LOGI(TAG, "loadRemote: ESP-Now Receiver (0x%02x) connected.", i2c);
      }
      else {
        TF_LOGW(TAG, "loadRemote: ESP-Now Receiver (0x%02x) not found.", i2c);
      }
    }
    else if (_esp_now_type == ESP_NOW_LOCAL) {
      if (_espnow.beginLocal()) {
        TF_LOGI(TAG, "loadRemote: ESP-Now initialized.");
      }
      else {
        TF_LOGW(TAG, "loadRemote: ESP-Now initialization failed.");
      }
    }

    _espnow.clearWhitelist();

    auto whitelist = yaml.findKeys(key_exist ? "hardware:esp_now:whitelist" : "hardware:i2c_esp_now:whitelist");
    if (whitelist != nullptr) {
      for (const auto& it : whitelist->asObjects()) {
        if (it.isString() == false) continue;
        
        uint8_t mac[6];
        if (parse_mac(it.asString().c_str(), mac)) {
          TF_LOGD(TAG, "loadRemote: whitelist added %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
          _espnow.addWhitelist(mac);
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

  color_map.insert({"color2_eyes", yaml.getString("my_protogen:color2_eyes", "#0000ff")});
  color_map.insert({"color2_nose", yaml.getString("my_protogen:color2_nose", "color2_eyes")});
  color_map.insert({"color2_mouth", yaml.getString("my_protogen:color2_mouth", "color2_eyes")});
  color_map.insert({"color2_side", yaml.getString("my_protogen:color2_side", "color2_eyes")});

  color_map.insert({"color2_right_eyes", yaml.getString("my_protogen:color2_right_eyes", "color2_eyes")});
  color_map.insert({"color2_right_nose", yaml.getString("my_protogen:color2_right_nose", "color2_nose")});
  color_map.insert({"color2_right_mouth", yaml.getString("my_protogen:color2_right_mouth", "color2_mouth")});
  color_map.insert({"color2_right_side", yaml.getString("my_protogen:color2_right_side", "color2_side")});

  color_map.insert({"color3_eyes", yaml.getString("my_protogen:color3_eyes", "#ff0000")});
  color_map.insert({"color3_nose", yaml.getString("my_protogen:color3_nose", "color3_eyes")});
  color_map.insert({"color3_mouth", yaml.getString("my_protogen:color3_mouth", "color3_eyes")});
  color_map.insert({"color3_side", yaml.getString("my_protogen:color3_side", "color3_eyes")});

  color_map.insert({"color3_right_eyes", yaml.getString("my_protogen:color3_right_eyes", "color3_eyes")});
  color_map.insert({"color3_right_nose", yaml.getString("my_protogen:color3_right_nose", "color3_nose")});
  color_map.insert({"color3_right_mouth", yaml.getString("my_protogen:color3_right_mouth", "color3_mouth")});
  color_map.insert({"color3_right_side", yaml.getString("my_protogen:color3_right_side", "color3_side")});

  if (!color_map.empty()) {
    auto set_color = [&color_map](const char* name, uint8_t personal_index, uint8_t index, uint32_t def_color) -> bool {
      std::string color_ref = name;

      for (int i = 0; i < 8; i++) {
        auto find = color_map.find(color_ref);
        if (find == color_map.end()) {
          break;
        }

        if (find->second[0] != 'c') {
          uint32_t color = parse_hex(find->second.c_str());
          Effect::setPersonalColor(personal_index, index, color);
          return true;
        }

        color_ref = find->second;
      }

      Effect::setPersonalColor(personal_index, index, def_color);
      return false;
    };

    set_color("color_eyes", 0, Effect::PC_EYES, 0x00ff00);
    set_color("color_nose", 0, Effect::PC_NOSE, 0x00ff00);
    set_color("color_mouth", 0, Effect::PC_MOUTH, 0x00ff00);
    set_color("color_side", 0, Effect::PC_SIDE, 0x00ff00);
    
    set_color("color_right_eyes", 0, Effect::PC_EYES + Effect::PC_MAX, 0x00ff00);
    set_color("color_right_nose", 0, Effect::PC_NOSE + Effect::PC_MAX, 0x00ff00);
    set_color("color_right_mouth", 0, Effect::PC_MOUTH + Effect::PC_MAX, 0x00ff00);
    set_color("color_right_side", 0, Effect::PC_SIDE + Effect::PC_MAX, 0x00ff00);
    
    set_color("color2_eyes", 1, Effect::PC_EYES, 0x0000ff);
    set_color("color2_nose", 1, Effect::PC_NOSE, 0x0000ff);
    set_color("color2_mouth", 1, Effect::PC_MOUTH, 0x0000ff);
    set_color("color2_side", 1, Effect::PC_SIDE, 0x0000ff);
    
    set_color("color2_right_eyes", 1, Effect::PC_EYES + Effect::PC_MAX, 0x0000ff);
    set_color("color2_right_nose", 1, Effect::PC_NOSE + Effect::PC_MAX, 0x0000ff);
    set_color("color2_right_mouth", 1, Effect::PC_MOUTH + Effect::PC_MAX, 0x0000ff);
    set_color("color2_right_side", 1, Effect::PC_SIDE + Effect::PC_MAX, 0x0000ff);
    
    set_color("color3_eyes", 2, Effect::PC_EYES, 0xff0000);
    set_color("color3_nose", 2, Effect::PC_NOSE, 0xff0000);
    set_color("color3_mouth", 2, Effect::PC_MOUTH, 0xff0000);
    set_color("color3_side", 2, Effect::PC_SIDE, 0xff0000);
    
    set_color("color3_right_eyes", 2, Effect::PC_EYES + Effect::PC_MAX, 0xff0000);
    set_color("color3_right_nose", 2, Effect::PC_NOSE + Effect::PC_MAX, 0xff0000);
    set_color("color3_right_mouth", 2, Effect::PC_MOUTH + Effect::PC_MAX, 0xff0000);
    set_color("color3_right_side", 2, Effect::PC_SIDE + Effect::PC_MAX, 0xff0000);
  }

  auto color_mode = COLOR_MODE.find(yaml.getString("my_protogen:color_mode", "personal"));
  PROTOGEN_COLOR_MODE pcm = (color_mode != COLOR_MODE.end()) ? color_mode->second : PCM_PERSONAL;
  
  Effect::setColorMode(pcm);
  
  uint32_t rainbow_single_speed = get_time_ms(yaml, "my_protogen:rainbow_single_speed", "20ms");
  Effect::setRainbowSingleSpeed(rainbow_single_speed);

  uint32_t rainbow_speed = get_time_ms(yaml, "my_protogen:rainbow_speed", "10ms");
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


size_t Toaster::loadEmotionsFromFS(const char* path, bool from_sd) {
  size_t e_total_count = 0;
  fs::FS* fs = &FFat;
#ifdef USE_SD
  if (from_sd) {
    fs = &SD;
  }
#endif

  File dir = fs->open(path);
  if (dir) {
    String filename = dir.getNextFileName();
  
    while (!filename.isEmpty()) {
      File sub_dir = fs->open(filename);
      if (sub_dir) {
        if (sub_dir.isDirectory()) {
          const char* p = strrchr(filename.c_str(), '/');
          if (p == nullptr || strcasecmp(p + 1, EMOTION_DEFAULT) != 0) {
            auto e_yaml = YamlNodeArray::fromFile((filename + "/emotions.yaml").c_str(), *fs);
            size_t e_count = loadEmotionEachYaml(e_yaml, filename.c_str(), from_sd);

            e_total_count += e_count;
            TF_LOGI(TAG, "%s%s - %d emotions loaded", from_sd ? "/sd" : "", filename.c_str(), e_count);
          }
        }

        sub_dir.close();
      }

      filename = dir.getNextFileName();
    }

    dir.close();
  }

  return e_total_count;
}


// Returns the string value of key, or nullptr if it is missing or not a string.
static const std::string* find_string(const YamlNodeArray& yaml, const char* key) {
  auto node = yaml.findKeys(key);
  return (node != nullptr && node->isString()) ? &node->asString() : nullptr;
}


bool Toaster::loadEmotions(const YamlNodeArray& yaml) {
  // The "default" emotion supplies the parts that other emotions leave out.
  // Even without it, the built-in emotions below are registered so _emotions is never empty.
  _emotion_default = {"", EMOTION_DEFAULT, "", "", "", "", "", "", false};
  bool is_default_loaded = false;

  auto emotions = yaml.findKeys("emotions");
  if (emotions != nullptr && emotions->isObject()) {
    for (const auto& it : emotions->asObjects()) {
      if (it.isObject() == false) {
        continue;
      }

      const auto& emotion = it.asObjects();
      auto name = find_string(emotion, "name");
      if (name == nullptr || *name != EMOTION_DEFAULT) {
        continue;
      }

      const std::pair<const char*, std::string*> parts[] = {
        {"eyes", &_emotion_default.eyes},
        {"nose", &_emotion_default.nose},
        {"mouth", &_emotion_default.mouth},
        {"special", &_emotion_default.special},
        {"special2", &_emotion_default.special2},
        {"side", &_emotion_default.side},
      };

      for (const auto& part : parts) {
        auto value = find_string(emotion, part.first);
        if (value != nullptr) {
          *part.second = *value;
        }
        else {
          TF_LOGW(TAG, "loadEmotions: emotion 'default' has no '%s', left empty", part.first);
        }
      }

      is_default_loaded = true;
      break;
    }
  }

  if (is_default_loaded == false) {
    TF_LOGE(TAG, "loadEmotions: emotion 'default' not found. check emotion.yaml");
  }

  _emotions.push_back({"", "blank", "", "", "blank", "", "", "blank", true});
  _emotions.push_back({"", "white", "", "", "white", "", "", "white", true});
  _emotions.push_back({"", "festive", "", "", "festive", "", "", "side_rainbow", false});
  _emotions.push_back({"", "clock", "", "", "clock", "", "", "side_default", false});
  _emotions.push_back({"", "dino", "", "", "dino", "", "", "side_rainbow", false});

  hud_emotions.clear();
  hud_emotions.push_back(HUDEmotions(EMOTION_DEFAULT));
  
  size_t e_count = loadEmotionEachYaml(yaml, DEFAULT_BASE_PATH, false);
  size_t e_total_count = e_count;
  TF_LOGI(TAG, "%d emotions loaded", e_count);

  e_total_count += loadEmotionsFromFS(EMOTIONS_DIR, false);
  if (_sd_use) {
    e_total_count += loadEmotionsFromFS(EMOTIONS_DIR, true);
  }

  TF_LOGI(TAG, "total %d emotions loaded!", e_total_count);

  addHUDEmotion("", "festive", "~(^w^)~");
  addHUDEmotion("", "clock", "clock");
  addHUDEmotion("", "dino", "dino");
  addHUDEmotion("", "blank", "Shutdown");

  clearEmptyHUDEmotions();

  return is_default_loaded;
}


size_t Toaster::loadEmotionEachYaml(const YamlNodeArray& yaml, const char* base_path, bool from_sd) {
  auto emotions = yaml.findKeys("emotions");
  if (emotions == nullptr || emotions->isObject() == false || emotions->asObjects().empty()) {
    return 0;
  }

  size_t count = 0;

  for (const auto& it : emotions->asObjects()) {
    if (it.isObject() == false) {
      continue;
    }

    auto& emotion = it.asObjects();
    
    auto name = find_string(emotion, "name");
    if (name == nullptr) continue;

    auto eyes = find_string(emotion, "eyes");
    auto nose = find_string(emotion, "nose");
    auto mouth = find_string(emotion, "mouth");
    auto special = find_string(emotion, "special");
    auto special2 = find_string(emotion, "special2");
    auto side = find_string(emotion, "side");

    if (*name == EMOTION_DEFAULT) {
      continue;
    }

    std::string dir;

    if (from_sd) {
      dir = "/sd";
    }
    dir += base_path;

    if (dir == DEFAULT_BASE_PATH) {
      dir.clear();
    }

    EMOTION_DATA data;
    data.base_path = dir;
    data.name = *name;
    data.eyes = (eyes == nullptr) ? _emotion_default.eyes : *eyes;
    data.nose = (nose == nullptr) ? _emotion_default.nose : *nose;
    data.mouth = (mouth == nullptr) ? _emotion_default.mouth : *mouth;
    data.special = (special == nullptr) ? _emotion_default.special : *special;
    data.special2 = (special2 == nullptr) ? _emotion_default.special2 : *special2;
    data.side = (side == nullptr) ? _emotion_default.side : *side;
    data.shuffle_exclude = false;

    int find = findEmotion(name->c_str());
    if (find >= 0) {
      if (from_sd) {
        // remove from the group of the emotion being replaced, before it is overwritten
        removeHUDEmotion(_emotions[find].base_path.c_str(), _emotions[find].name.c_str());
        _emotions[find] = data;
        TF_LOGI(TAG, "emotion (%s) already exist. replaced.", name->c_str());
      }
      else {
        TF_LOGW(TAG, "emotion (%s) already exist. ignored.", name->c_str());
        continue;
      }
    }
    else {
      _emotions.push_back(data);
    }
    
    addHUDEmotion(dir.c_str(), name->c_str());

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

  return true;
}


};
