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
        TF_LOGI(TAG, "Set Shutdown");
        Protogen.setEmotion("blank");
        break;
      case '1':
        TF_LOGI(TAG, "Set Normal");
        Protogen.setEmotion("normal");
        break;
      case '2':
        TF_LOGI(TAG, "Set Badass");
        Protogen.setEmotion("badass");
        break;
      case '3':
        TF_LOGI(TAG, "Set Grin");
        Protogen.setEmotion("grin");
        break;
      case '4':
        TF_LOGI(TAG, "Set Worry");
        Protogen.setEmotion("worry");
        break;
      case '5':
        TF_LOGI(TAG, "Set Love");
        Protogen.setEmotion("love");
        break;
      case '6':
        TF_LOGI(TAG, "Set Confusion");
        Protogen.setEmotion("confusion");
        break;
      case '7':
        TF_LOGI(TAG, "Set Question");
        Protogen.setEmotion("qmark");
        break;
      case '8':
        TF_LOGI(TAG, "Set Unyuu");
        Protogen.setEmotion("unyuu");
        break;
      case '9':
        TF_LOGI(TAG, "Set Bean");
        Protogen.setEmotion("bean");
        break;
      case 'P': case 'p':
        TF_LOGI(TAG, "Set BSOD");
        Protogen.setEmotion("bsod");
        break;
      case 'W': case 'w':
        TF_LOGI(TAG, "Set White");
        Protogen.setEmotion("white");
        break;
      case ';':
        TF_LOGI(TAG, "Set Festive");
        Protogen.setEmotion("festive");
        break;
      case '\'':
        TF_LOGI(TAG, "Set Loading");
        Protogen.setEmotion("loading");
        break;
      case 'Q': case 'q':
        Protogen._boopsensor.setEmulation(!Protogen._boopsensor.getEmulation());
        TF_LOGI(TAG, "Boop emulation: %s", Protogen._boopsensor.getEmulation() ? "on" : "off");
        break;

      case 'Z': case 'z':
        TF_LOGI(TAG, "Set Color Original");
        Protogen.setColorMode(PCM_ORIGINAL);
        break;
      case 'X': case 'x':
        TF_LOGI(TAG, "Set Color Personal");
        Protogen.setColorMode(PCM_PERSONAL);
        break;
      case 'C': case 'c':
        TF_LOGI(TAG, "Set Color Rainbow single");
        Protogen.setColorMode(PCM_RAINBOW_SINGLE);
        break;
      case 'V': case 'v':
        TF_LOGI(TAG, "Set Color Rainbow");
        Protogen.setColorMode(PCM_RAINBOW);
        break;
      case 'B': case 'b':
        TF_LOGI(TAG, "Set Color Gradation");
        Protogen.setColorMode(PCM_GRADATION);
        break;

      case 'M': case 'm':
        if (Protogen._boopsensor.isError()) {
          TF_LOGW(TAG, "Cannot start debug mode because Boop sensor is disabled.");
        }
        else {
          Protogen._boopsensor.setDebug(!Protogen._boopsensor.getDebug());
        }
        break;
      
      case 'A': case 'a':
        Protogen.setStaticMode(!Protogen.getStaticMode());
        TF_LOGI(TAG, "Static mode: %s", Protogen.getStaticMode() ? "on" : "off");
        break;
      case '!':
        serial_text_mode = true;
        break;
      }
    }
    else {
      if (ch == '\r' || ch == '\n') {
        const char* read = serial_buffer_read();

        if (strncasecmp(read, "b ", 2) == 0) {
          float brightness = atoi(read + 1) / 100.0f;
          Protogen.refreshAutoBrightness(brightness);
          TF_LOGI(TAG, "Set brightness: %.3f", brightness);
        }
        else if (strcasecmp(read, "reset") == 0 || strcasecmp(read, "restart") == 0 || strcasecmp(read, "reboot") == 0) {
          TF_LOGI(TAG, "soft reset");
          ESP.restart();
        }
        else if (strcasecmp(read, "mac") == 0) {
          uint8_t mac[6];
          esp_read_mac(mac, ESP_MAC_WIFI_STA);
          TF_LOGI(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
        else if (strcasecmp(read, "boop") == 0) {
          Protogen._boopsensor.setEnabled(true);
          TF_LOGI(TAG, "Boop sensor enabled");
        }
        else if (strcasecmp(read, "noboop") == 0) {
          Protogen._boopsensor.setEnabled(false);
          TF_LOGI(TAG, "Boop sensor disabled");
        }
        else if (strncasecmp(read, "set ", 4) == 0) {
          TF_LOGI(TAG, "Set %s", read + 4);
          Protogen.setEmotion(read + 4);
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
