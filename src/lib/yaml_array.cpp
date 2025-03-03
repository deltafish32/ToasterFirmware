#include "yaml_array.h"
#include "yaml_node.h"
#include <string>
#include <memory>


namespace toaster {

static const size_t YAML_INDENT = 2;
static const char* WHITESPACE = " \t\n\r\f\v";



static bool is_yaml_comment(const char* str) {
  const char* ptr = str;

  while (*ptr != 0) {
    char ch = *ptr;

    if (ch == '#') {
      return true;
    }
    if (ch != ' ') {
      return false;
    }

    ++ptr;
  }

  // empty strings are also treated as comments.
  return true;
}


static bool is_yaml_array(const char* str) {
  const char* ptr = str;

  while (*ptr == ' ') {
    ++ptr;
  }

  return (ptr[0] == '-' && ptr[1] == ' ');
}


static std::string grab_line(const char* str) {
  const char* ptr = str;

  while (*ptr != 0 && *ptr != '\r' && *ptr != '\n') {
    ++ptr;
  }

  return std::string(str, ptr - str);
}


static const char* find_newline(const char* str) {
  const char* ptr = str;

  while (*ptr == '\r' || *ptr == '\n') {
    ++ptr;
  }

  return ptr;
}


static size_t grab_indent(const char* str) {
  const char* ptr = str;

  while (*ptr == ' ') {
    ++ptr;
  }

  return ptr - str;
}


static std::string parse_value(const char* str) {
  int mode = 0;
  bool end = false;
  const char* ptr = str;

  while (*ptr == ' ') {
    ++ptr;
  }

  const char* ptr2 = ptr;
  std::string prefix;

  while (*ptr != 0 && end == false) {
    switch (mode) {
    case 0:
      if (*ptr == '\'') {
        prefix = std::string(str, ptr - str);
        ptr2 = ptr + 1;
        mode = 100;
      }
      else if (*ptr == '\"') {
        prefix = std::string(str, ptr - str);
        ptr2 = ptr + 1;
        mode = 200;
      }
      else if (*ptr == '#') {
        end = true;
        break;
      }
      break;
    case 100: // '
      if (*ptr == '\'') {
        end = true;
        break;
      }
      break;
    case 200: // "
      if (*ptr == '\"') {
        end = true;
        break;
      }
      break;
    }
    ++ptr;
  }

  while (*ptr == ' ') {
    --ptr;
  }

  return prefix + std::string(ptr2, (end) ? (ptr - ptr2 - 1) : (ptr - ptr2));
}


static uint8_t parse_key_value(const char* str, std::string& out_key, std::string& out_value) {
  const char* ptr = str;
  const char* ptr2;
  bool is_array = false;

  if (is_yaml_array(str)) { // array
    is_array = true;
    str += 2;
    ptr = str;
  }

  while (*ptr != 0 && *ptr != ':') {
    ++ptr;
  }

  if (*ptr != ':') {
    if (is_array == false) {
      return YamlNode::YAML_NONE;
    }
    else {
      out_key = "";
      out_value = parse_value(str);

      return YamlNode::YAML_ARRAY;
    }
  }

  ptr2 = ptr;

  ++ptr;

  if (*ptr == ' ') {
    out_key = std::string(str, ptr2 - str);
    out_value = parse_value(ptr + 1);

    return is_array ? YamlNode::YAML_ARRAY : YamlNode::YAML_STRING;
  }
  else {
    if (!is_yaml_comment(ptr)) {
      //return YamlNode::YAML_NONE;
      out_key = "";
      out_value = parse_value(str);

      return YamlNode::YAML_ARRAY;
    }

    out_key = std::string(str, ptr2 - str);
    out_value = "";

    return YamlNode::YAML_OBJECT; // or array
  }
}


static std::string parse_key(const char* str) {
  const char* ptr = str;
  
  while (*ptr != 0 && *ptr != ':') {
    ++ptr;
  }

  return std::string(str, ptr - str);
}


void YamlNodeArray::pushString(const char* key, const char* str) {
  push_back({ key, str });
}


void YamlNodeArray::pushArray(const char* key) {
  push_back({ key, YamlNode::YAML_ARRAY });
}


void YamlNodeArray::pushObject(const char* key) {
  push_back({ key, YamlNode::YAML_OBJECT });
}


YamlNode* YamlNodeArray::find(const char* key) {
   if (*key == 0) {
     return nullptr;
   }

   for (auto& it : *this) {
     if (it.key == key) {
       return &it;
     }
   }

   return nullptr;
}


YamlNode* YamlNodeArray::findKeys(const char* keys) {
  return _findKeys(this, keys);
}


const YamlNode* YamlNodeArray::find(const char* key) const {
   if (*key == 0) {
     return nullptr;
   }

   for (const auto& it : *this) {
     if (it.key == key) {
       return &it;
     }
   }

   return nullptr;
}


const YamlNode* YamlNodeArray::findKeys(const char* keys) const {
  return _findKeys((YamlNodeArray*)this, keys);
}


std::string YamlNodeArray::getString(const char* keys, const char* def_str) const {
  char* array_index = nullptr;
  const YamlNode* node = _findKeys((YamlNodeArray*)this, keys, &array_index);
  if (node == nullptr) {
    return def_str;
  }

  if (node->isString()) {
    return node->asString();
  }

  return def_str;
}


int YamlNodeArray::getInt(const char* keys, int def_value) const {
  std::string str = getString(keys, "");
  return (str.empty()) ? def_value : atoi(str.c_str());
}


float YamlNodeArray::getFloat(const char* keys, float def_value) const {
  std::string str = getString(keys, "");
  return (str.empty()) ? def_value : atoff(str.c_str());
}


std::string YamlNodeArray::toString(const char* newline, const char* indent) const {
  std::string str;
  _toString(*this, str, 0, newline, indent);
  return str;
}


YamlNodeArray YamlNodeArray::fromBytes(const char* data, size_t data_size) {
  YamlNodeArray yaml;
  std::vector<YamlNodeArray*> stack;

  stack.push_back(&yaml);

  const char* ptr = data;
  int recent_depth = 0;
  bool recent_array = false;

  auto stack_rewind = [&stack](int n = 1) {
    for (int i = 0; i < n; i++) {
      stack.pop_back();
    }
  };

  while (*ptr != 0) {
    std::string line = grab_line(ptr);
    ptr += line.length();
    ptr = find_newline(ptr);

    if (is_yaml_comment(line.c_str())) {
      continue;
    }

    size_t indent = grab_indent(line.c_str());
    if (indent % YAML_INDENT != 0) {
      return YamlNodeArray("indent odd error");
    }

    int line_depth = (int)(indent / YAML_INDENT);
    if (line_depth - recent_depth > 1) {
      return YamlNodeArray("indent error");
    }

    const char* line_start = line.c_str() + indent;
    bool is_array = is_yaml_array(line_start);

    stack_rewind(recent_depth - line_depth);

    recent_depth = line_depth;

    std::string key;
    std::string value;
    uint8_t type = parse_key_value(line_start, key, value);

    if (type == YamlNode::YAML_STRING) {
      stack.back()->pushString(key.c_str(), value.c_str());
    }
    else if (type == YamlNode::YAML_OBJECT) {
      stack.back()->pushObject(key.c_str());
      stack.back()->back().setObject();
      stack.push_back(&stack.back()->back().asObjects());
      recent_array = false;
    }
    else if (type == YamlNode::YAML_ARRAY) {
      char index_str[16];
      sprintf(index_str, "%d", stack.back()->size());

      if (key.empty()) {
        stack.back()->pushString(index_str, value.c_str());
      }
      else {
        stack.back()->pushObject(index_str);
        stack.back()->back().setObject();
        stack.push_back(&stack.back()->back().asObjects());

        stack.back()->pushString(key.c_str(), value.c_str());
        recent_array = true;
      }
    }
    else {
      return YamlNodeArray("format error");
    }
  }

  return yaml;
}


#ifdef USE_ARDUINO
YamlNodeArray YamlNodeArray::fromFile(const char* filename, fs::FS& filesystem) {
  if (!filesystem.exists(filename)) {
    return YamlNodeArray("file does not exist.");
  }

  File file = filesystem.open(filename, FILE_READ);
  if (!file) {
    return YamlNodeArray("file open failed.");
  }

  size_t size = file.size();

  std::shared_ptr<char[]> buffer(new char[size + 1]);
  if (buffer.get() == nullptr) {
    file.close();
    return YamlNodeArray("memory allocation failed.");
  }
  file.readBytes(buffer.get(), size);
  buffer.get()[size] = 0;
  file.close();

  return fromBytes(buffer.get(), size);
}
#else
YamlNodeArray YamlNodeArray::fromFile(const char* filename) {
  FILE* f = fopen(filename, "rb");
  if (f == nullptr) {
    return YamlNodeArray("file open failed.");
  }

  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  fseek(f, 0, SEEK_SET);

  std::shared_ptr<char[]> buffer(new char[size + 1]);

  fread(buffer.get(), 1, size, f);
  buffer.get()[size] = 0;

  fclose(f);

  return fromBytes(buffer.get(), size);
}
#endif


YamlNode* YamlNodeArray::_findKeys(YamlNodeArray* yaml, const char* keys, char** array_index) {
  while (*keys != 0) {
    std::string key = parse_key(keys);
    keys += key.length();
    keys += ((*keys == ':') ? 1 : 0);

    YamlNode* node = yaml->find(key.c_str());
    if (node == nullptr) {
        break;
    }
    else if (node->isString()) {
      if (*keys != 0) {
        break;
      }
      return node;
    }
    else if (node->isObject()) {
      if (*keys == 0) {
        return node;
      }
      yaml = &node->asObjects();
    }
    else {
      break;
    }
  }

  return nullptr;
}


void YamlNodeArray::_toString(const YamlNodeArray& nodes, std::string& str, int depth, const char* newline, const char* indent, bool is_array) {
  for (const auto& it : nodes) {
    if (is_array) {
      is_array = false;
    }
    else {
      for (int i = 0; i < depth; i++) str += indent;
    }

    switch (it.getType()) {
    case YamlNode::YAML_OBJECT:
      if (isdigit(it.key[0])) {
        str += "- ";
        _toString(it.asObjects(), str, depth + 1, newline, indent, true);
        is_array = false;
      }
      else {
        str += it.key;
        str += ':';
        str += newline;
        _toString(it.asObjects(), str, depth + 1, newline, indent, false);
      }
      break;
    case YamlNode::YAML_STRING:
      if (isdigit(it.key[0])) {
        str += "- ";
      }
      else {
        str += it.key;
        str += ": ";
      }
      str += it.asString();
      str += newline;
      break;
    }
  }
}

};
