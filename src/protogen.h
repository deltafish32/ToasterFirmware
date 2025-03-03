#pragma once
#include "config/configure.h"


#include "lib/worker.h"
#include "lib/linear_calibrate.h"
#include "lib/yaml.h"
#include "protogen_color_mode.h"
#include "effects/effect_base.h"
#include "huds/hud.h"


#include "hal/display/hub75_display.h"
#include "hal/display/neopixel_display.h"
#include "hal/controller/serial_debug.h"
#include "hal/sensor/lightsensor.h"
#include "hal/sensor/boopsensor.h"
#include "hal/sensor/thermometer.h"
#include "hal/sensor/rtc.h"
#include "hal/controller/i2c_esp_now.h"
#include "hal/controller/ir_remote.h"


namespace toaster {


enum {
  EFFECT_MAX = 5,

  // 0: single click
  // 1: click 2 times
  // 2: click 3 times
  // 3: long click
  SHORTCUT_CLICK_COUNT_MAX = 3,

  RTC_FORMAT_HM = 1,
  RTC_FORMAT_HMS,
  RTC_FORMAT_HM_NO_BLINKING,
  RTC_FORMAT_HMS_NO_BLINKING,
};


enum {
  SIDE_PANEL_STRIP = 0,
  SIDE_PANEL_CIRCLE,
};

extern const std::vector<std::pair<float, float>> SIDE_PANEL_CIRCLE_DATA;


class Toaster : public Worker {
public:
  Toaster();

public:
  virtual bool begin();
  virtual void loop();

protected:
  virtual bool work();
  virtual bool workPerSecond();

public:
  bool setEmotion(const char* emotion);
  bool setEmotion(size_t index);
  const char* getEmotion() const {
    return _emotions[_emotion_index].name.c_str();
  }

  const char* getEmotionGroup() const;

  size_t getEmotionIndex() const {
    return _emotion_index;
  }

  const size_t getEmotionCount() const {
    return _emotions.size();
  }

  void shuffleEmotion();

  bool setNextEmotion();
  void displayEmotionList();

public:
  bool loadShortcut(const char* shortcut);
  bool setEmotionShortcut(uint8_t hand, uint8_t finger, uint8_t action);

  const std::vector<std::string>& getShortcutList() const {
    return _shortcut_list;
  }

  const char* getShortcut() const {
    return _shortcut_list[_shortcut_index].c_str();
  }

protected:
  void setEffect(int index, const char* new_effect, const char* base_path);
  void setSideEffect(const char* new_effect, const char* base_path);

public:
  bool getStaticMode() const {
    return Effect::getStaticMode();
  }

  void setStaticMode(bool mode) {
    Effect::setStaticMode(mode);
  }

  PROTOGEN_COLOR_MODE getColorMode() const {
    return Effect::getColorMode();
  }

  void setColorMode(PROTOGEN_COLOR_MODE mode) {
    Effect::setColorMode(mode);
  }
  
  void refreshAutoBrightness();
  void refreshAutoBrightness(float brightness);

  void toast();

public:
  void syncLock(TickType_t xBlockTime = portMAX_DELAY);
  void syncUnlock();

  bool isBlank() const;

  const char* getVersion() const {
    return _tf_version;
  }

  bool isFestiveFace() const {
    return _festive_face;
  }

  bool isAdaptiveFps() const;

  bool isRGB565() const {
    return _use_rgb565;
  }

  bool isGame() const;

  const char* getTimeStr() const;

  uint8_t getSidePanelType() const {
    return _side_panel_type;
  }

protected:
  static const char* _tf_version;
  bool _init{false};
  SemaphoreHandle_t _interruptSemaphore{nullptr};
  uint32_t _sync_count{0};
  uint8_t _sync_core{0};

public:
  static const char* DEFAULT_BASE_PATH;

public:
  SerialDebug _serialdebug;
  Hub75Display _display;
  NeoPixelDisplay _side_display;
  LightSensor _lightsensor;
  BoopSensor _boopsensor;
  Thermometer _thermometer;
  HeadUpDisplay _hud;
  I2CESPNowRemote _i2c_espnow;
  IRRemote _ir_remote;
  RTC _rtc;

protected:
  int _version{0};
  bool _side_display_use{false};
  bool _hud_use{false};
  bool _lightsensor_use{false};
  bool _boopsensor_use{false};
  bool _thermometer_use{false};
  bool _espnow_use{false};
  bool _ir_remote_use{false};
  bool _rtc_use{false};
  uint8_t _rtc_format{0};

protected:
  uint32_t _hub75_fps{60};
  uint32_t _hud_fps{30};
  bool _festive_face{true};
  bool _use_rgb565{true};
  uint8_t _side_panel_type{0};
  float _default_brightness{1.0f};
  float _side_brightness_rate{1.0f};
  LinearCalibrate<float, float> _ls_auto;
  float _ls_recent{0};
  float _ls_hys{0};

protected:
  Effect* _effect[EFFECT_MAX]{nullptr,};
  Effect* _side_effect{nullptr};
  size_t _emotion_index{0};

protected:
  // LR, Finger, Clicks (0 to 2, 3: Long-click)
  std::string _shortcuts[2][3][SHORTCUT_CLICK_COUNT_MAX + 1];
  std::vector<std::string> _shortcut_list;
  size_t _shortcut_index{0};

protected:
  typedef struct _EMOTION_DATA {
    std::string base_path;
    std::string name;
    std::string eyes;
    std::string nose;
    std::string mouth;
    std::string special;
    std::string special2;
    std::string side;
    bool shuffle_exclude;
  } EMOTION_DATA;
  EMOTION_DATA _emotion_default;
  std::vector<EMOTION_DATA> _emotions;
  
  std::vector<int> _shuffle_deck;

protected:
  bool isEmotionExist(const char* name) const;
  void addHUDEmotion(const char* base_path, const char* emotion, const char* display_name = nullptr);

protected:
  bool loadHub75(const YamlNodeArray& yaml);
  bool loadSidePanel(const YamlNodeArray& yaml);
  bool loadHUD(const YamlNodeArray& yaml);
  bool loadPhotoresistor(const YamlNodeArray& yaml);
  bool loadLightSensor(const YamlNodeArray& yaml);
  bool loadBoopSensor(const YamlNodeArray& yaml);
  bool loadThermometer(const YamlNodeArray& yaml);
  bool loadRTC(const YamlNodeArray& yaml);
  bool loadRemote(const YamlNodeArray& yaml);
  bool loadPersonality(const YamlNodeArray& yaml);
  void loadGradation(uint8_t index, const YamlNode* node);
  bool loadEmotions(const YamlNodeArray& yaml);
  size_t loadEmotionEachYaml(const YamlNodeArray& yaml, const char* base_path);
  bool loadDefaultEmotion(const YamlNodeArray& yaml);
  bool loadNEC(const YamlNodeArray& yaml);
  bool loadShortcutList();

};


extern Toaster Protogen;

};
