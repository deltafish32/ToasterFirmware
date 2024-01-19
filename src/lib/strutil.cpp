#include "strutil.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>


namespace toaster {

std::vector<std::string> my_split(const std::string& str, const std::string& delim) {
  std::vector<std::string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delim, prev);
    if (pos == std::string::npos) pos = str.length();
    std::string token = str.substr(prev, pos - prev);
    trim(token);
    if (!token.empty()) tokens.push_back(token);
    prev = pos + delim.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}

uint32_t parse_time_str(const char* str) {
  char* ptr;
  double n = strtod(str, &ptr);

  if (strcmp(ptr, "d") == 0) {
    return (uint32_t)round(n * 86400000);
  }

  if (strcmp(ptr, "h") == 0) {
    return (uint32_t)round(n * 3600000);
  }

  if (strcmp(ptr, "min") == 0) {
    return (uint32_t)round(n * 60000);
  }

  if (strcmp(ptr, "s") == 0) {
    return (uint32_t)round(n * 1000);
  }

  if (strcmp(ptr, "ms") == 0) {
    return (uint32_t)round(n);
  }

  return 0;
}


static uint32_t hex2dec(char ch) {
  return (ch >= '0' && ch <= '9') ? (ch - '0') : 
    (ch >= 'A' && ch <= 'F') ? (ch - 'A' + 10) : 
    (ch >= 'a' && ch <= 'f') ? (ch - 'a' + 10) : 0;
}


uint32_t parse_hex(const char* str) {
  const char* ptr = str;
  uint32_t value = 0;

  if (*ptr == 0) {
    return 0;
  }

  if (ptr[0] == '0' && (ptr[1] == 'X' || ptr[1] == 'x')) {
    ptr += 2;

    while (*ptr != 0) {
      value *= 16;
      value += hex2dec(*ptr);
      ++ptr;
    }
  }
  
  if (ptr[0] == '#') {
    ptr += 1;

    if (strlen(ptr) == 6) {
      value = ((hex2dec(ptr[0]) << 4) | (hex2dec(ptr[1]) << 0) | (hex2dec(ptr[2]) << 12) | (hex2dec(ptr[3]) << 8) | (hex2dec(ptr[4]) << 20) | (hex2dec(ptr[5]) << 16));
    }
  }

  return value;
}


bool parse_bool(const char* str) {
  if (strcmp(str, "true") == 0 || strcmp(str, "yes") == 0 || strcmp(str, "on") == 0 || strcmp(str, "enable") == 0) {
    return true;
  }

  return (atoi(str) != 0);  
  // if (strcmp(str, "false") == 0 || strcmp(str, "no") == 0 || strcmp(str, "off") == 0 || strcmp(str, "disable") == 0) {
  //   return false;
  // }

  // return false;
}


bool parse_separate(const char* str, const char* token, std::string& left, std::string& right) {
  const char* ptr = strstr(str, token);

  if (ptr == nullptr) {
    return false;
  }

  left = std::string(str, ptr - str);
  right = std::string(ptr + strlen(token));

  return true;
}


};
