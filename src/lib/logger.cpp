#include "logger.h"
#include <stdarg.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>


namespace toaster {

volatile uint8_t tf_log_level = (TF_LOG_MAX_LEVEL < TF_LOG_LEVEL_DEBUG) ? TF_LOG_MAX_LEVEL : TF_LOG_LEVEL_DEBUG;


static const char* const LEVEL_NAMES[] = {
  "none", "error", "warn", "info", "debug", "verbose",
};

static const size_t LEVEL_COUNT = sizeof(LEVEL_NAMES) / sizeof(LEVEL_NAMES[0]);

// Message color after the tag color (\e[0m: default)
static const char* const LEVEL_COLORS[] = {
  "\e[0m", "\e[31m", "\e[33m", "\e[32m", "\e[0m", "\e[90m",
};


void tf_log_set_level(tf_log_level_t level) {
  tf_log_level = (level < LEVEL_COUNT) ? level : TF_LOG_LEVEL_VERBOSE;
}


tf_log_level_t tf_log_get_level() {
  return (tf_log_level_t)tf_log_level;
}


const char* tf_log_level_to_string(tf_log_level_t level) {
  return (level < LEVEL_COUNT) ? LEVEL_NAMES[level] : "unknown";
}


bool tf_log_level_from_string(const char* str, tf_log_level_t* level) {
  if (str == nullptr || level == nullptr) {
    return false;
  }

  if (isdigit((unsigned char)str[0]) && str[1] == '\0') {
    int value = str[0] - '0';
    if (value < (int)LEVEL_COUNT) {
      *level = (tf_log_level_t)value;
      return true;
    }
    return false;
  }

  for (size_t i = 0; i < LEVEL_COUNT; i++) {
    if (strcasecmp(str, LEVEL_NAMES[i]) == 0) {
      *level = (tf_log_level_t)i;
      return true;
    }
  }

  if (strcasecmp(str, "warning") == 0) {
    *level = TF_LOG_LEVEL_WARN;
    return true;
  }

  return false;
}


// Build a line in a single buffer and write it at once so that logs from multiple tasks do not interleave.
// The buffer is static and guarded by a mutex, so it is safe even in tasks with small stacks.
void tf_log_write(tf_log_level_t level, const char* tag, const char* format, ...) {
  static const size_t LOG_BUFFER_SIZE = 512;
  static const char TRUNCATED[] = "...";
  static const char SUFFIX[] = "\e[0m\r\n";
  static char buffer[LOG_BUFFER_SIZE];
  static StaticSemaphore_t mutex_buffer;
  static SemaphoreHandle_t mutex = xSemaphoreCreateMutexStatic(&mutex_buffer);

  if (level >= LEVEL_COUNT) {
    level = TF_LOG_LEVEL_VERBOSE;
  }

  timer_us_t tick_us = Timer::get_micros();

  xSemaphoreTake(mutex, portMAX_DELAY);

  // Reserve space for the truncation mark and the suffix.
  const size_t body_limit = LOG_BUFFER_SIZE - (sizeof(TRUNCATED) - 1) - sizeof(SUFFIX);

  int len = snprintf(buffer, body_limit, "[%5lld.%06d] \e[36m%s: %s",
    tick_us / 1000000, (int)(tick_us % 1000000), (tag != nullptr) ? tag : "", LEVEL_COLORS[level]);
  if (len < 0) {
    len = 0;
  }

  bool truncated = false;
  if ((size_t)len >= body_limit) {
    len = body_limit - 1;
    truncated = true;
  }
  else {
    va_list args;
    va_start(args, format);
    int body_len = vsnprintf(buffer + len, body_limit - len, format, args);
    va_end(args);

    if (body_len > 0) {
      if ((size_t)(len + body_len) >= body_limit) {
        len = body_limit - 1;
        truncated = true;
      }
      else {
        len += body_len;
      }
    }
  }

  if (truncated) {
    memcpy(buffer + len, TRUNCATED, sizeof(TRUNCATED) - 1);
    len += sizeof(TRUNCATED) - 1;
  }

  memcpy(buffer + len, SUFFIX, sizeof(SUFFIX) - 1);
  len += sizeof(SUFFIX) - 1;

  Serial.write((const uint8_t*)buffer, len);

  xSemaphoreGive(mutex);
}

};
