#include "effect_parser.h"
#include <string.h>
#include <map>

#define FILESYSTEM SPIFFS
#include <SPIFFS.h>

#include "effect_base.h"
#include "lib/strutil.h"


namespace toaster {

static const std::map<std::string, uint8_t> methods = {
  {"none", Effect::SM_NONE},
  {"draw", Effect::SM_DRAW},
  {"draw_90", Effect::SM_DRAW_90},
  {"draw_180", Effect::SM_DRAW_180},
  {"draw_270", Effect::SM_DRAW_270},
};

static const std::map<std::string, uint8_t> colors = {
  {"original", Effect::PC_ORIGINAL},
  {"eyes", Effect::PC_EYES},
  {"nose", Effect::PC_NOSE},
  {"mouth", Effect::PC_MOUTH},
  {"side", Effect::PC_SIDE},
};

static const std::map<std::string, uint8_t> modes = {
  {"single", Effect::DM_SINGLE},
  {"copy", Effect::DM_COPY},
  {"mirror", Effect::DM_MIRROR},
  {"mirror_only_offset", Effect::DM_MIRROR_ONLY_OFFSET},
};


bool EffectParser::open(const char* filename) {
  release();

  YamlNodeArray yaml = YamlNodeArray::fromFile(filename, SPIFFS);

  _version = atoi(yaml.getString("version", "0").c_str());

  if (_version != 1) {
    _error = "version mismatch";
    return false;
  }

  auto images = yaml.findKeys("images");
  if (images == nullptr || images->isObject() == false) {
    _error = "key [images] does not exist or format error";
    return false;
  }

  _images.clear();
  for (const auto& it : images->asObjects()) {
    _images.push_back(it.asString());
  }

  auto sequences = yaml.findKeys("sequences");
  if (sequences == nullptr || sequences->isObject() == false) {
    _error = "key [sequences] does not exist or format error";
    return false;
  }

  for (const auto& it : sequences->asObjects()) {
    auto split_result = my_split(it.asString(), ",");
    
    if (split_result.empty()) {
      continue;
    }

    DRAW_SEQUENCE ds;

    auto method = methods.find(split_result[0].c_str());
    if (method == methods.end()) {
    _error = "failed to parse [method] in [sequences]";
      return false;
    }
    ds.method = method->second;

    switch (ds.method) {
    case Effect::SM_NONE:
      if (split_result.size() < 2) {
        continue;
      }

      ds.display_time = (uint32_t)(atof(split_result[1].c_str()) * 1000);
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270:
      if (split_result.size() < 7) {
        _error = "not enough arguments in [sequences]";
        return false;
      }

      ds.image = atoi(split_result[1].c_str());

      auto color = colors.find(split_result[2].c_str());
      if (color == colors.end()) {
        _error = "failed to parse [color] in [sequences]";
        return false;
      }
      ds.color = color->second;

      auto mode = modes.find(split_result[3].c_str());
      if (mode == modes.end()) {
        _error = "failed to parse [mode] in [sequences]";
        return false;
      }
      ds.mode = mode->second;

      ds.offset_x = atoi(split_result[4].c_str());
      ds.offset_y = atoi(split_result[5].c_str());
      ds.display_time = (uint32_t)(atof(split_result[6].c_str()) * 1000);
      break;
    }

    _sequences.push_back(ds);
  }

  return true;
}

};
