#pragma once
#include <Arduino.h>
#include "timer.h"


namespace toaster {

enum tf_log_level_t : uint8_t {
  TF_LOG_LEVEL_NONE = 0,
  TF_LOG_LEVEL_ERROR,
  TF_LOG_LEVEL_WARN,
  TF_LOG_LEVEL_INFO,
  TF_LOG_LEVEL_DEBUG,
  TF_LOG_LEVEL_VERBOSE,
};

// Runtime log level (changed by log_level in config.yaml or the loglevel serial command)
extern volatile uint8_t tf_log_level;

inline bool tf_log_enabled(tf_log_level_t level) {
  return level <= tf_log_level;
}

void tf_log_set_level(tf_log_level_t level);
tf_log_level_t tf_log_get_level();
const char* tf_log_level_to_string(tf_log_level_t level);
bool tf_log_level_from_string(const char* str, tf_log_level_t* level);

void tf_log_write(tf_log_level_t level, const char* tag, const char* format, ...)
  __attribute__((format(printf, 3, 4)));

};


// Compile-time maximum log level (0: none to 5: verbose)
// Logs above this level are removed from the code, and their arguments are not evaluated.
// e.g. -DTF_LOG_MAX_LEVEL=3 in build_flags (include up to info)
#ifndef TF_LOG_MAX_LEVEL
#define TF_LOG_MAX_LEVEL 4
#endif

#define TF_LOG_IMPL(LEVEL, TAG, ...) do { \
    if ((int)(LEVEL) <= TF_LOG_MAX_LEVEL && toaster::tf_log_enabled(LEVEL)) { \
      toaster::tf_log_write(LEVEL, TAG, __VA_ARGS__); \
    } \
  } while (0)

#define TF_LOGE(TAG, ...)       TF_LOG_IMPL(toaster::TF_LOG_LEVEL_ERROR,   TAG, __VA_ARGS__)
#define TF_LOGW(TAG, ...)       TF_LOG_IMPL(toaster::TF_LOG_LEVEL_WARN,    TAG, __VA_ARGS__)
#define TF_LOGI(TAG, ...)       TF_LOG_IMPL(toaster::TF_LOG_LEVEL_INFO,    TAG, __VA_ARGS__)
#define TF_LOGD(TAG, ...)       TF_LOG_IMPL(toaster::TF_LOG_LEVEL_DEBUG,   TAG, __VA_ARGS__)
#define TF_LOGV(TAG, ...)       TF_LOG_IMPL(toaster::TF_LOG_LEVEL_VERBOSE, TAG, __VA_ARGS__)
