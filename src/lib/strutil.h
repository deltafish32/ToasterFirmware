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
bool parse_mac(const char* str, uint8_t* mac);

// Append formatted text at buf[len], never writing past buf_size.
// len is advanced and clamped so repeated calls stay safe after truncation.
void appendf(char* buf, size_t buf_size, size_t& len, const char* fmt, ...) __attribute__((format(printf, 4, 5)));

// Format a user-supplied frame path (e.g. "png/%03d.png") with a frame number.
// fmt may contain at most one integer conversion (%d/%i/%u with optional flags,
// width and precision) plus "%%". Returns false if fmt is invalid or the result
// does not fit in out_size.
bool format_frame_path(char* out, size_t out_size, const char* fmt, int frame);

};
