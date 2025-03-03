#pragma once


typedef struct _ESP_NOW_DATA_BASE {
  uint8_t mac[6];
  uint8_t sign[2];
} ESP_NOW_DATA_BASE;


typedef struct _ESP_NOW_DATA_REMOTE : _ESP_NOW_DATA_BASE {
  uint8_t data;
  uint8_t spare;
  uint16_t battery;
} ESP_NOW_DATA_REMOTE;
