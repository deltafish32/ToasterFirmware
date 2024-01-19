#pragma once
#include <vector>
#include <string>


#include "yaml_array.h"


namespace toaster {

class YamlNode {
public:
  std::string key;

public:
  YamlNode();
  YamlNode(const YamlNode& other);
  YamlNode(const char* new_key, const char* new_str = "");
  YamlNode(const char* new_key, uint8_t new_type);

  ~YamlNode() {
    release();
  }

  void release();

  YamlNode& operator=(const YamlNode& other);

  uint8_t getType() const {
    return _type;
  }

  bool isString() const {
    return (_type == YAML_STRING);
  }

  bool isObject() const {
    return (_type == YAML_OBJECT);
  }

public: // YAML_STRING
  void setString(const char* str);

  const std::string& asString() const {
    return *_value;
  }

public: // YAML_OBJECT
  void setObject();

  void pushObject(const YamlNode& other);

  YamlNodeArray& asObjects() {
    return *_sub;
  }

  const YamlNodeArray& asObjects() const {
    return *_sub;
  }

public:
  enum {
    YAML_NONE = 0,
    YAML_STRING,
    YAML_ARRAY,
    YAML_OBJECT,
  };

protected:
  uint8_t _type{ 0 };
  union {
    void* _ptr{ nullptr };
    std::string* _value;
    YamlNodeArray* _sub;
  };

};

};
