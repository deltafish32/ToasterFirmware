#pragma once
#include "timer.h"


#define TF_LOG_TIMESTAMP()      { timer_us_t tick_us = Timer::get_micros(); Serial.printf("[%5lld.%06d] ", tick_us / 1000000, (int)(tick_us % 1000000)); }

#define TF_LOGE(TAG, STR...)    { TF_LOG_TIMESTAMP(); Serial.printf("\033[36m%s: \033[31m", TAG); Serial.printf(STR); Serial.println("\033[0m"); }
#define TF_LOGW(TAG, STR...)    { TF_LOG_TIMESTAMP(); Serial.printf("\033[36m%s: \033[33m", TAG); Serial.printf(STR); Serial.println("\033[0m"); }
#define TF_LOGI(TAG, STR...)    { TF_LOG_TIMESTAMP(); Serial.printf("\033[36m%s: \033[32m", TAG); Serial.printf(STR); Serial.println("\033[0m"); }
#define TF_LOGD(TAG, STR...)    { TF_LOG_TIMESTAMP(); Serial.printf("\033[36m%s: \033[0m",  TAG); Serial.printf(STR); Serial.println(); }
