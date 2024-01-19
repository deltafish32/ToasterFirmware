#include <Arduino.h>
#include "serial_debug.h"

#include "protogen.h"
#include "effects/effects.h"
#include "huds/hud.h"
#include "lib/logger.h"


namespace toaster {

static const char* TAG = "SerialDebug";
static const int SERIAL_BUFFER_MAX = 1024;


SerialDebug::SerialDebug() {
}

bool SerialDebug::begin() {
  Serial.begin(115200);
  
  return true;
}

void SerialDebug::loop() {
  while (Serial.available()) {
    char ch = Serial.read();

    if (serial_text_mode == false) {
      switch (ch) {
      case 'H': case 'h':
        Protogen._hud.pressKey('H');
        break;
      case 'J': case 'j':
        Protogen._hud.pressKey('J');
        break;
      case 'K': case 'k':
        Protogen._hud.pressKey('K');
        break;
      case 'L': case 'l':
        Protogen._hud.pressKey('L');
        break;
      case 'S': case 's':
        Protogen._hud.pressKey('S');
        break;
      case 'D': case 'd':
        Protogen._hud.pressKey('D');
        break;
      case 'F': case 'f':
        Protogen._hud.pressKey('F');
        break;

      case '0':
        TF_LOG(TAG, "Set Shutdown");
        Protogen.setEmotion("blank");
        break;
      case '1':
        TF_LOG(TAG, "Set Normal");
        Protogen.setEmotion("normal");
        break;
      case '2':
        TF_LOG(TAG, "Set Badass");
        Protogen.setEmotion("badass");
        break;
      case '3':
        TF_LOG(TAG, "Set Grin");
        Protogen.setEmotion("grin");
        break;
      case '4':
        TF_LOG(TAG, "Set Worry");
        Protogen.setEmotion("worry");
        break;
      case '5':
        TF_LOG(TAG, "Set Love");
        Protogen.setEmotion("love");
        break;
      case '6':
        TF_LOG(TAG, "Set Confusion");
        Protogen.setEmotion("confusion");
        break;
      case '7':
        TF_LOG(TAG, "Set Question");
        Protogen.setEmotion("qmark");
        break;
      case '8':
        TF_LOG(TAG, "Set Unyuu");
        Protogen.setEmotion("unyuu");
        break;
      case '9':
        TF_LOG(TAG, "Set Bean");
        Protogen.setEmotion("bean");
        break;
      case 'P': case 'p':
        TF_LOG(TAG, "Set BSOD");
        Protogen.setEmotion("bsod");
        break;
      case '-':
        TF_LOG(TAG, "Set Align");
        Protogen.setEmotion("align");
        break;
      case 'W': case 'w':
        TF_LOG(TAG, "Set White");
        Protogen.setEmotion("white");
        break;
      case ';':
        TF_LOG(TAG, "Set Festive");
        Protogen.setEmotion("festive");
        break;
      case '\'':
        TF_LOG(TAG, "Set Loading");
        Protogen.setEmotion("loading");
        break;

      case 'Z': case 'z':
        TF_LOG(TAG, "Set Color Original");
        Protogen.setColorMode(PCM_ORIGINAL);
        break;
      case 'X': case 'x':
        TF_LOG(TAG, "Set Color Personal");
        Protogen.setColorMode(PCM_PERSONAL);
        break;
      case 'C': case 'c':
        TF_LOG(TAG, "Set Color Rainbow single");
        Protogen.setColorMode(PCM_RAINBOW_SINGLE);
        break;
      case 'V': case 'v':
        TF_LOG(TAG, "Set Color Rainbow");
        Protogen.setColorMode(PCM_RAINBOW);
        break;
      case 'B': case 'b':
        TF_LOG(TAG, "Set Color Gradation");
        Protogen.setColorMode(PCM_GRADATION);
        break;
      
      case 'A': case 'a':
        Protogen.setStaticMode(!Protogen.getStaticMode());
        TF_LOG(TAG, "Static mode: %s\n", Protogen.getStaticMode() ? "on" : "off");
        break;
      case '!':
        serial_text_mode = true;
        break;
      }
    }
    else {
      if (ch == '\r' || ch == '\n') {
        const char* read = serial_buffer_read();

        if (read[0] == 'b') {
          uint8_t value = (uint8_t)(atoi(read + 1) / 100.0f * 255);
          Protogen._display.setBrightness(value);
          Protogen._side_display.setBrightness(value);
          TF_LOG(TAG, "Set brightness: %.3f\n", Protogen._display.getBrightness() / 255.0f);
        }

        _serial_buffer.clear();

        serial_text_mode = false;
      }
      else if (ch == 8) { // backspace
        if (!_serial_buffer.empty()) {
          _serial_buffer.pop_back();
        }
      }
      else {
        while (_serial_buffer.size() >= SERIAL_BUFFER_MAX) {
          _serial_buffer.erase(_serial_buffer.begin());
        }

        _serial_buffer.push_back(ch);
      }
    }

  }

}


const char* SerialDebug::serial_buffer_read() {
  static char str[SERIAL_BUFFER_MAX + 1];
  char* p = str;

  for (auto it : _serial_buffer) {
    *p++ = it;
  }
  *p = 0;

  return str;
}

};
