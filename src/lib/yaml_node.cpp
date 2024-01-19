#include "yaml_node.h"


namespace toaster {

YamlNode::YamlNode() {
}


YamlNode::YamlNode(const YamlNode& other) {
  *this = other;
}


YamlNode::YamlNode(const char* new_key, const char* new_str) : key(new_key) {
  setString(new_str);
}


YamlNode::YamlNode(const char* new_key, uint8_t new_type) : key(new_key) {
  switch (new_type) {
  case YAML_STRING:
    setString("");
    break;
  case YAML_OBJECT:
    setObject();
    break;
  }
}


void YamlNode::release() {
  if (_ptr != nullptr) {
    switch (_type) {
    case YAML_STRING: delete _value; break;
    case YAML_OBJECT: delete _sub; break;
    }
    _ptr = nullptr;
  }
}


YamlNode& YamlNode::operator=(const YamlNode& other) {
  key = other.key;

  switch (other._type) {
  case YAML_STRING:
    setString(other._value->c_str());
    break;
  case YAML_OBJECT:
    setObject();
    *_sub = *other._sub;
    break;
  }

  return *this;
}


void YamlNode::setString(const char* str) {
  if (_type == YAML_STRING) {
    *_value = str;
  }
  else {
    release();
    _type = YAML_STRING;
    _value = new std::string(str);
  }
}


void YamlNode::setObject() {
  if (_type != YAML_OBJECT) {
    release();
    _type = YAML_OBJECT;
    _sub = new YamlNodeArray();
  }
}


void YamlNode::pushObject(const YamlNode& other) {
  _sub->pushObject(key.c_str());
  _sub->back() = other;
}

};
