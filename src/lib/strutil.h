#pragma once
#include <stdint.h>
#include <vector>
#include <string>


namespace toaster {

static inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v") {
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}

static inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v") {
  s.erase(0, s.find_first_not_of(t));
  return s;
}

static inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v") {
  return ltrim(rtrim(s, t), t);
}

std::vector<std::string> my_split(const std::string& str, const std::string& delim);

uint32_t parse_time_str(const char* str);
uint32_t parse_hex(const char* str);
bool parse_bool(const char* str);
bool parse_separate(const char* str, const char* token, std::string& left, std::string& right);

};
