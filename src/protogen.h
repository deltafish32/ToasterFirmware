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
#include "hal/controller/i2c_esp_now.h"
#include "hal/controller/ir_remote.h"


namespace toaster {

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

protected:
  void setEffect(const char* new_effect1, const char* new_effect2 = "", const char* new_effect3 = "");
  void setSideEffect(const char* new_effect1);

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

  bool isBlank() const {
    return (_effect[0] == nullptr || _effect[0]->isBlank()) && 
      (_effect[1] == nullptr || _effect[1]->isBlank()) && 
      (_effect[2] == nullptr || _effect[2]->isBlank());
  }
  
  const char* getVersion() const {
    return _tf_version;
  }

  bool isFestiveFace() const {
    return _festive_face;
  }

protected:
  static const char* _tf_version;
  bool _init{false};
  SemaphoreHandle_t _interruptSemaphore{nullptr};
  uint32_t _sync_count{0};
  uint8_t _sync_core{0};

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

protected:
  int _version{0};
  bool _side_display_use{false};
  bool _hud_use{false};
  bool _lightsensor_use{false};
  bool _boopsensor_use{false};
  bool _thermometer_use{false};
  bool _espnow_use{false};
  bool _ir_remote_use{false};

protected:
  uint32_t _hub75_fps{60};
  uint32_t _hud_fps{30};
  bool _festive_face{true};
  float _default_brightness{1.0f};
  float _side_brightness_rate{1.0f};
  LinearCalibrate<float, float> _ls_auto;
  float _ls_recent{0};
  float _ls_hys{0};

protected:
  Effect* _effect[3]{nullptr,};
  Effect* _side_effect{nullptr};

protected:
  typedef struct _EMOTION_DATA {
    std::string name;
    std::string eyes;
    std::string nose;
    std::string mouth;
    std::string side;
  } EMOTION_DATA;
  EMOTION_DATA _emotion_default;
  std::vector<EMOTION_DATA> _emotions;

protected:
  bool loadHub75(const YamlNodeArray& yaml);
  bool loadSidePanel(const YamlNodeArray& yaml);
  bool loadHUD(const YamlNodeArray& yaml);
  bool loadPhotoresistor(const YamlNodeArray& yaml);
  bool loadLightSensor(const YamlNodeArray& yaml);
  bool loadBoopSensor(const YamlNodeArray& yaml);
  bool loadThermometer(const YamlNodeArray& yaml);
  bool loadRemote(const YamlNodeArray& yaml);
  bool loadPersonality(const YamlNodeArray& yaml);
  bool loadEmotions(const YamlNodeArray& yaml);
  bool loadNEC(const YamlNodeArray& yaml);

};


extern Toaster Protogen;

};
