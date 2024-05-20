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

    if (_serial_text_mode == false) {
      switch (ch) {
      // https://en.wikipedia.org/wiki/ANSI_escape_code
      case '\e':
        if (!Serial.available()) { // escape
          pressKey(ch);
        }
        else {
          char ch2 = Serial.read();
          if (ch2 == '[' && Serial.available()) {
            char ch3 = Serial.read();
            switch (ch3) {
            case 'A':
              pressKey(VK_UP, KM_KEYCODE);
              break;
            case 'B':
              pressKey(VK_DOWN, KM_KEYCODE);
              break;
            case 'C':
              pressKey(VK_RIGHT, KM_KEYCODE);
              break;
            case 'D':
              pressKey(VK_LEFT, KM_KEYCODE);
              break;
            }
          }
        }
        break;

      case '!':
      case '\n':
        _serial_text_mode = true;
        _serial_buffer.clear();
        _cursor = 0;
        Serial.print("> ");
        break;

      default:
        pressKey(ch);
        break;
      }
    }
    else {
      if (ch == '\n') {
        Serial.println();

        processCommand(serial_buffer_read());

        _serial_buffer.clear();
        _serial_text_mode = false;
      }
      else if (ch == '\b') { // backspace
        if (!_serial_buffer.empty() && _cursor > 0) {
          _serial_buffer.erase(_serial_buffer.begin() + _cursor - 1);
          --_cursor;
          Serial.print((_cursor == _serial_buffer.size()) ? "\b \b" : "\b");
          printAfterCursor(true);
        }
      }
      else if (ch == '\e') {
        if (!Serial.available()) { // escape
          _serial_buffer.clear();
          _serial_text_mode = false;

          Serial.println();
        }
        else {
          char ch2 = Serial.read();
          if (ch2 == '[' && Serial.available()) {
            char ch3 = Serial.read();
            if (isdigit(ch3)) {
              int vt = ch3 - '0';
              while (Serial.available()) {
                char ch4 = Serial.read();
                if (isdigit(ch4)) {
                  vt *= 10;
                  vt += ch4 - '0';
                }
                else if (ch4 == '~') {
                  if (vt == 3) { // delete
                    if (_cursor < _serial_buffer.size()) {
                      _serial_buffer.erase(_serial_buffer.begin() + _cursor);
                      if (_cursor == _serial_buffer.size()) {
                        Serial.print(" \b");
                      }
                      printAfterCursor(true);
                    }
                  }
                  break;
                }
                else {
                  break;
                }
              }
            }
            else if (ch3 == 'C') { // right
              if (_cursor < _serial_buffer.size()) {
                ++_cursor;
                Serial.print("\e[C");
              }
              break;
            }
            else if (ch3 == 'D') { // left
              if (_cursor > 0) {
                --_cursor;
                Serial.print("\e[D");
              }
              break;
            }
            else if (ch3 == 'H') { // home
              int prev_cursor = _cursor;
              _cursor = 0;
              if (prev_cursor - _cursor > 0) {
                Serial.printf("\e[%dD", prev_cursor - _cursor);
              }
            }
            else if (ch3 == 'F') { // end
              int prev_cursor = _cursor;
              _cursor = _serial_buffer.size();
              if (_cursor - prev_cursor > 0) {
                Serial.printf("\e[%dC", _cursor - prev_cursor);
              }
            }
          }
        }
      }
      else if (ch == '\r' || (ch & 0x80) != 0) {
        // throw
      }
      else {
        if (_serial_buffer.size() < SERIAL_BUFFER_MAX) {
          _serial_buffer.insert(_serial_buffer.begin() + _cursor, ch);
          ++_cursor;

          Serial.print(ch);
          printAfterCursor();
        }
      }
    }
  }

}


bool SerialDebug::pressKey(uint16_t key, uint8_t mode) {
  if (mode == KM_ASCII) {
    switch (key) {
    case 'F': case 'f':
    case 'D': case 'd':
    case 'S': case 's':
    case 'A': case 'a':
    case 'J': case 'j':
    case 'K': case 'k':
    case 'L': case 'l':
    case ';': case ':':
      Protogen._hud.pressKey(key);
      return true;

    case '\e':
      Serial.println("esc");
      return true;
      break;

    case '0':
      TF_LOGI(TAG, "Set Shutdown");
      Protogen.setEmotion("blank");
      return true;
      break;
    case '1':
      TF_LOGI(TAG, "Set Normal");
      Protogen.setEmotion("normal");
      return true;
      break;
    case '2':
      TF_LOGI(TAG, "Set Badass");
      Protogen.setEmotion("badass");
      return true;
      break;
    case '3':
      TF_LOGI(TAG, "Set Grin");
      Protogen.setEmotion("grin");
      return true;
      break;
    case '4':
      TF_LOGI(TAG, "Set Worry");
      Protogen.setEmotion("worry");
      return true;
      break;
    case '5':
      TF_LOGI(TAG, "Set Love");
      Protogen.setEmotion("love");
      return true;
      break;
    case '6':
      TF_LOGI(TAG, "Set Confusion");
      Protogen.setEmotion("confusion");
      return true;
      break;
    case '7':
      TF_LOGI(TAG, "Set Question");
      Protogen.setEmotion("qmark");
      return true;
      break;
    case '8':
      TF_LOGI(TAG, "Set Unyuu");
      Protogen.setEmotion("unyuu");
      return true;
      break;
    case '9':
      TF_LOGI(TAG, "Set Bean");
      Protogen.setEmotion("bean");
      return true;
      break;
    case 'P': case 'p':
      TF_LOGI(TAG, "Set BSOD");
      Protogen.setEmotion("bsod");
      return true;
      break;
    case 'W': case 'w':
      TF_LOGI(TAG, "Set White");
      Protogen.setEmotion("white");
      return true;
      break;
    case 'O': case 'o':
      TF_LOGI(TAG, "Set Festive");
      Protogen.setEmotion("festive");
      return true;
      break;
    case '\'':
      TF_LOGI(TAG, "Set Loading");
      Protogen.setEmotion("loading");
      return true;
      break;
    case 'Q': case 'q':
      Protogen._boopsensor.setEmulation(!Protogen._boopsensor.getEmulation());
      TF_LOGI(TAG, "Boop emulation: %s", Protogen._boopsensor.getEmulation() ? "on" : "off");
      return true;
      break;

    case 'Z': case 'z':
      TF_LOGI(TAG, "Set Color Original");
      Protogen.setColorMode(PCM_ORIGINAL);
      return true;
      break;
    case 'X': case 'x':
      TF_LOGI(TAG, "Set Color Personal");
      Protogen.setColorMode(PCM_PERSONAL);
      return true;
      break;
    case 'C': case 'c':
      TF_LOGI(TAG, "Set Color Rainbow single");
      Protogen.setColorMode(PCM_RAINBOW_SINGLE);
      return true;
      break;
    case 'V': case 'v':
      TF_LOGI(TAG, "Set Color Rainbow");
      Protogen.setColorMode(PCM_RAINBOW);
      return true;
      break;
    case 'B': case 'b':
      TF_LOGI(TAG, "Set Color Gradation");
      Protogen.setColorMode(PCM_GRADATION);
      return true;
      break;

    case 'M': case 'm':
      if (Protogen._boopsensor.isError()) {
        TF_LOGW(TAG, "Cannot start debug mode because Boop sensor is disabled.");
      }
      else {
        Protogen._boopsensor.setDebug(!Protogen._boopsensor.getDebug());
      }
      return true;
      break;

    case '\t':
      Protogen.setNextEmotion();
      TF_LOGI(TAG, "Set Next Emotion: %s", Protogen.getEmotion());
      return true;
      break;

    case '\\':
      Protogen._hud.setDithering(!Protogen._hud.getDithering());
      TF_LOGI(TAG, "Set HUD dithering: %s", Protogen._hud.getDithering() ? "true" : "false");
      return true;
      break;
    
    case '!':
    case '\n':
      _serial_text_mode = true;
      Serial.print("> ");
      return true;
      break;
    }
  }
  else { //if (mode == KM_KEYCODE) {
    switch (key) {
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
      Protogen._hud.pressKey(key, mode);
      return true;
      break;
    }
  }

  return false;
}


typedef struct _COMMAND {
  const char* cmd;
  const char* desc;
  std::function<void(const char*)> func;
} COMMAND;


static const auto reset_func = [](const char*) { TF_LOGI(TAG, "soft reset"); ESP.restart(); };

static const size_t getCommandCount();

static const COMMAND COMMAND_LIST[] = {
  { "help", "Display information about builtin commands.", [](const char*) {
    size_t command_count = getCommandCount();
    for (size_t i = 0; i < command_count; i++) {
      Serial.printf("%-16s: %s\n", COMMAND_LIST[i].cmd, COMMAND_LIST[i].desc);
    }
    Serial.println();
  } },
  { "reset|restart|reboot", "Soft reset.",                            reset_func },
  { "b",                    "Set display brightness.",                [](const char* param) { float brightness = atoi(param) / 100.0f; Protogen.refreshAutoBrightness(brightness); TF_LOGI(TAG, "Set brightness: %.3f", brightness); } },
  { "ls|emotions",          "Show list of loaded emotions.",          [](const char*) { Protogen.displayEmotionList(); } },
  { "set",                  "Change to the entered emotion.",         [](const char* param) { TF_LOGI(TAG, "Set %s", param); Protogen.setEmotion(param); } },
  { "mac",                  "Display mac address.",                   [](const char*) { uint8_t mac[6]; esp_read_mac(mac, ESP_MAC_WIFI_STA); TF_LOGI(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); } },
  { "boop",                 "Enable boop sensor.",                    [](const char*) { Protogen._boopsensor.setEnabled(true); TF_LOGI(TAG, "Boop sensor enabled"); } },
  { "noboop",               "Disable boop sensor.",                   [](const char*) { Protogen._boopsensor.setEnabled(false);  TF_LOGI(TAG, "Boop sensor disabled"); } },
  { "dithering",            "Enable HUD dithering.",                  [](const char*) { Protogen._hud.setDithering(true); TF_LOGI(TAG, "HUD dithering enabled"); } },
  { "nodithering",          "Disable HUD dithering.",                 [](const char*) { Protogen._hud.setDithering(false); TF_LOGI(TAG, "HUD dithering disabled"); } },
};

static const size_t getCommandCount() {
  return sizeof(COMMAND_LIST) / sizeof(COMMAND_LIST[0]);
}


bool SerialDebug::processCommand(const char* cmd) {
  const char* param = strchr(cmd, ' ');
  std::string cmd_name(cmd, (param != nullptr) ? param - cmd : strlen(cmd));

  if (cmd_name.empty()) {
    return true;
  }

  if (param != nullptr) {
    ++param;
  }

  for (auto& it : COMMAND_LIST) {
    const char* find_start = it.cmd;
    while (1) {
      const char* find_end = strchr(find_start, '|');
      size_t find_len = (find_end != nullptr) ? find_end - find_start : strlen(find_start);

      if (cmd_name.length() == find_len && strncasecmp(cmd_name.c_str(), find_start, find_len) == 0) {
        if (it.func != nullptr) {
          it.func(param != nullptr ? param : "");
        }
        return true;
      }

      if (find_end == nullptr) {
        break;
      }

      find_start = find_end + 1;
    }
    
  }

  TF_LOGW(TAG, "command (%s) not found.", cmd_name.c_str());

  return false;
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


void SerialDebug::printAfterCursor(bool erase) {
  int del = _serial_buffer.size() - _cursor + (del ? 1 : 0);
  if (del <= 0) {
    return;
  }

  for (size_t i = _cursor; i < _serial_buffer.size(); i++) {
    Serial.print(_serial_buffer[i]);
  }

  if (erase) {
    Serial.print(' ');
    ++del;
  }

  Serial.printf("\e[%dD", del);
}


};
