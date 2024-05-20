#pragma once
#include <vector>

#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>


namespace toaster {

class IRRemote {
public:
  IRRemote();
  ~IRRemote();

  bool begin(uint16_t pin, bool debug = false);
  void loop();

public:
  void clearEvents() {
    _event_emotions.clear();
    _event_keypresses.clear();
  }

  void addNEC_Emotion(uint32_t nec_code, const char* emotion) {
    _event_emotions.push_back({nec_code, emotion});
  }

  void addNEC_Keypress(uint32_t nec_code, char key, uint8_t mode) {
    _event_keypresses.push_back({nec_code, key, mode});
  }

protected:
  bool processNEC_Emotion(uint32_t nec_code);
  bool processNEC_Keypress(uint32_t nec_code);

protected:
  bool _init{false};
  bool _debug{false};
  IRrecv* _irrecv{nullptr};
  decode_results _recent_result;

protected:
  typedef struct _IR_EVENT_EMOTION {
    uint32_t nec_code;
    std::string emotion;
  } IR_EVENT_EMOTION;
  std::vector<IR_EVENT_EMOTION> _event_emotions;

  typedef struct _IR_EVENT_KEYPRESS {
    uint32_t nec_code;
    char key;
    uint8_t mode;
  } IR_EVENT_KEYPRESS;
  std::vector<IR_EVENT_KEYPRESS> _event_keypresses;

};

};
