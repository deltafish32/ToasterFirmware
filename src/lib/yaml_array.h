#pragma once
#include <vector>
#include <string>

#ifdef USE_ARDUINO
#include <Arduino.h>
#include <FS.h>
#endif


namespace toaster {

class YamlNode;


class YamlNodeArray : public std::vector<YamlNode> {
public:
  YamlNodeArray() {}

protected:
  YamlNodeArray(const char* error) : _error(error) {}

public:
  void pushString(const char* key, const char* str);
  void pushArray(const char* key);
  void pushObject(const char* key);
  void pushPending(const char* key);

public:
  YamlNode* find(const char* key);
  const YamlNode* find(const char* key) const;

  YamlNode* findKeys(const char* keys);
  const YamlNode* findKeys(const char* keys) const;

  bool isKeysExist(const char* keys) const {
    return findKeys(keys) != nullptr;
  }
  std::string getString(const char* keys, const char* def_str = "") const;
  int getInt(const char* keys, int def_value = 0) const;
  float getFloat(const char* keys, float def_value = 0.0f) const;

public:
  void trace() {
    printf("%s\n", toString().c_str());
  }

  std::string toString(const char* newline = "\n", const char* indent = "  ") const;

public:
  bool isError() const {
    return !_error.empty();
  }

  const char* getLastError() const {
    return _error.c_str();
  }

public:
  static YamlNodeArray fromBytes(const char* data, size_t data_size);
#ifdef USE_ARDUINO
  static YamlNodeArray fromFile(const char* filename, fs::FS& filesystem);
#else
  static YamlNodeArray fromFile(const char* filename);
#endif

protected:
  std::string _error;

protected:
  static YamlNode* _findKeys(YamlNodeArray* yaml, const char* keys, char** array_index = nullptr);
  static void _toString(const YamlNodeArray& nodes, std::string& str, int depth, const char* newline, const char* indent, bool is_array = false);

};

};


#include <iostream>

static std::ostream& operator<<(std::ostream& os, toaster::YamlNodeArray& yaml) {
  return os << yaml.toString().c_str();
}
