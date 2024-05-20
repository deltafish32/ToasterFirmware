#include "ir_remote.h"

#include "lib/logger.h"
#include "protogen.h"


namespace toaster {

static const char* TAG = "IRRemote";


IRRemote::IRRemote() {
}


IRRemote::~IRRemote() {
  if (_irrecv != nullptr) {
    delete _irrecv;
    _irrecv = nullptr;
  }
}


bool IRRemote::begin(uint16_t pin, bool debug) {
  if (_init) {
    return false;
  }

  _debug = debug;
  _irrecv = new IRrecv(pin, 1024, 15, true);

  if (_irrecv == nullptr) {
    return false;
  }

  _irrecv->enableIRIn(true);

  int high_count = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(pin) == HIGH) {
      ++high_count;
    }
    delay(1);
  }

  // TF_LOGD(TAG, "%d", high_count);

  if (high_count < 9) {
    return false;
  }

  _init = true;

  return true;
}


static uint32_t bit_reverse(uint32_t x) {
  x = ((x >>  1) & 0x55555555) | ((x & 0x55555555) <<  1);
  x = ((x >>  2) & 0x33333333) | ((x & 0x33333333) <<  2);
  x = ((x >>  4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) <<  4);
  x = ((x >>  8) & 0x00ff00ff) | ((x & 0x00ff00ff) <<  8);
  x = ((x >> 16) & 0x0000ffff) | ((x & 0x0000ffff) << 16);
  return x;
}


void IRRemote::loop() {
  if (!_init) {
    return;
  }

  decode_results results;
  
  if (_irrecv->decode(&results)) {
    // if (_debug) {
    //   uint32_t now = millis();
    //   Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
      
    //   if (results.overflow)
    //     Serial.printf(D_WARN_BUFFERFULL "\n", 1024);
      
    //   Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");

    //   Serial.print(resultToHumanReadableBasic(&results));
      
    //   Serial.println(resultToSourceCode(&results));
    //   Serial.println();
    // }

    if (results.decode_type == NEC) {
      if (results.repeat == false) {
        _recent_result = results;

        // The order of transferring bits was corrected from MSB to LSB in accordance with the NEC standard.
        uint32_t nec_code = bit_reverse(results.value);
        uint16_t nec_address = nec_code & 0xffff;
        uint16_t nec_command = (nec_code >> 16) & 0xffff;

        TF_LOGI(TAG, "on_nec: address: 0x%04x, command: 0x%04x (full: 0x%08x)", nec_address, nec_command, nec_code);

        if (!processNEC_Emotion(nec_code)) {
          if (!processNEC_Keypress(nec_code)) {
          }
        }

      }
    }
  }
}


bool IRRemote::processNEC_Emotion(uint32_t nec_code) {
  for (const auto& it : _event_emotions) {
    if (it.nec_code == nec_code) {
      Protogen.setEmotion(it.emotion.c_str());
      return true;
    }
  }

  return false;
}


bool IRRemote::processNEC_Keypress(uint32_t nec_code) {
  for (const auto& it : _event_keypresses) {
    if (it.nec_code == nec_code) {
      Protogen._hud.pressKey(it.key, it.mode);
      return true;
    }
  }

  return false;
}


};
