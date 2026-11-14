#include "hud_usb_storage.h"

#include <Arduino.h>


namespace toaster {

HUDUsbStorage hud_usb_storage;


// Shown as transferring if a read/write happened within this time.
static const uint32_t ACTIVITY_MS = 1000;


static void formatBytes(char* buf, size_t size, uint64_t bytes) {
  snprintf(buf, size, "%u.%uM", (unsigned)(bytes / (1024 * 1024)), (unsigned)((bytes % (1024 * 1024)) * 10 / (1024 * 1024)));
}


// last_ms is recorded as millis() | 1, so it can be 1ms ahead of now; compare as a signed difference.
static bool isRecent(uint32_t last_ms, uint32_t now) {
  return last_ms != 0 && (int32_t)(now - last_ms) < (int32_t)ACTIVITY_MS;
}


void HUDUsbStorage::draw(Adafruit_SSD1306& oled, const UsbStorage::Status& status) {
  uint32_t now = millis();
  bool reading = isRecent(status.last_read_ms, now);
  bool writing = isRecent(status.last_write_ms, now);

  oled.clearDisplay();

  // Title
  setFont(oled);
  oled.setCursor(0, 0);
  writeCenter(oled, "USB MODE");

  // Animation
  drawAnimation(oled, 0, 16, oled.width(), 24, status, reading, writing);

  // Transfer amount
  if (status.state == UsbStorage::STATE_CONNECTED || status.state == UsbStorage::STATE_FINISHING) {
    char sz_read[12];
    char sz_write[12];
    char sz[32];
    formatBytes(sz_read, sizeof(sz_read), status.read_bytes);
    formatBytes(sz_write, sizeof(sz_write), status.write_bytes);
    snprintf(sz, sizeof(sz), "R %s  W %s", sz_read, sz_write);

    oled.setFont(nullptr);
    oled.setCursor(0, 40);
    writeCenter(oled, sz);
  }

  // Status
  const char* text = "";
  switch (status.state) {
  case UsbStorage::STATE_WAITING:
  case UsbStorage::STATE_CONNECTED:
    // Alternate with a notice when entered because of an error
    if ((status.error_boot || status.sd_error) && (now / 2000) % 2 == 0) {
      text = status.error_boot ? "Config error" : "SD card error";
    }
    else if (status.state == UsbStorage::STATE_WAITING) {
      text = "Connect to PC";
    }
    else {
      text = writing ? "Writing" : (reading ? "Reading" : "Connected");
    }
    break;
  case UsbStorage::STATE_FINISHING:
    text = "Waiting...";
    break;
  case UsbStorage::STATE_RESTARTING:
    text = "Restarting";
    break;
  }

  setFont(oled);
  oled.setCursor(0, 48);
  writeCenter(oled, text);

  oled.display();

  ++_frame;
}


void HUDUsbStorage::drawAnimation(Adafruit_SSD1306& oled, int16_t x, int16_t y, int16_t w, int16_t h, const UsbStorage::Status& status, bool reading, bool writing) {
  if (status.state == UsbStorage::STATE_RESTARTING) {
    return;
  }

  // PC (left)
  const int16_t pc_x = x + 4;
  const int16_t pc_y = y + 2;
  oled.drawRect(pc_x, pc_y, 22, 15, SSD1306_WHITE);
  oled.fillRect(pc_x + 9, pc_y + 15, 4, 3, SSD1306_WHITE);
  oled.drawFastHLine(pc_x + 5, pc_y + 18, 12, SSD1306_WHITE);

  // Toaster (right)
  const int16_t toaster_x = x + w - 28;
  const int16_t toaster_y = y + 6;
  oled.drawRoundRect(toaster_x, toaster_y, 24, 15, 4, SSD1306_WHITE);
  oled.drawFastHLine(toaster_x + 5, toaster_y + 3, 6, SSD1306_WHITE);
  oled.drawFastHLine(toaster_x + 13, toaster_y + 3, 6, SSD1306_WHITE);

  // Bread pops up while writing
  if (writing) {
    int16_t pop = (_frame % 4 < 2) ? 3 : 1;
    oled.drawRect(toaster_x + 6, toaster_y - pop, 4, pop + 1, SSD1306_WHITE);
    oled.drawRect(toaster_x + 14, toaster_y - pop, 4, pop + 1, SSD1306_WHITE);
  }

  // Cable (dotted line)
  const int16_t cable_x1 = pc_x + 26;
  const int16_t cable_x2 = toaster_x - 4;
  const int16_t cable_len = cable_x2 - cable_x1;
  const int16_t cable_y = y + 12;
  const int16_t DOT_GAP = 6;
  const int16_t SPEED = 2;

  if (status.state == UsbStorage::STATE_WAITING) {
    // Cable broken in the middle, both ends blinking
    const int16_t gap = 16;
    bool blink = (_frame / 5) % 2 == 0;
    for (int16_t dx = 0; dx <= cable_len; dx += DOT_GAP) {
      int16_t px = cable_x1 + dx;
      bool near_gap = std::abs(px - (cable_x1 + cable_len / 2)) < gap;
      if (near_gap) {
        continue;
      }
      bool end_dot = std::abs(px - (cable_x1 + cable_len / 2)) < gap + DOT_GAP;
      if (end_dot && !blink) {
        continue;
      }
      oled.fillRect(px, cable_y, 2, 2, SSD1306_WHITE);
    }
    return;
  }

  // Connected: dots flow PC -> toaster when writing, toaster -> PC when reading
  int16_t offset = 0;
  if (writing) {
    offset = (_frame * SPEED) % DOT_GAP;
  }
  else if (reading) {
    offset = DOT_GAP - 1 - (_frame * SPEED) % DOT_GAP;
  }

  for (int16_t dx = offset; dx <= cable_len; dx += DOT_GAP) {
    oled.fillRect(cable_x1 + dx, cable_y, 2, 2, SSD1306_WHITE);
  }

  // Show a progress bar on the PC screen while transferring
  if (reading || writing) {
    int16_t bar = (_frame % 8) * 2 + 2;
    oled.fillRect(pc_x + 2, pc_y + 6, std::min<int16_t>(bar, 18), 3, SSD1306_WHITE);
  }
}

};
