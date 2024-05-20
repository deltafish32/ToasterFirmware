#include "effect_parser.h"
#include <string.h>
#include <map>

#include <FFat.h>

#include "effect_base.h"
#include "lib/strutil.h"
#include "lib/logger.h"


namespace toaster {

static const char* TAG = "EffectParser";


static const std::map<std::string, uint8_t> methods = {
  {"none", Effect::SM_NONE},
  {"draw", Effect::SM_DRAW},
  {"draw_90", Effect::SM_DRAW_90},
  {"draw_180", Effect::SM_DRAW_180},
  {"draw_270", Effect::SM_DRAW_270},
  {"video", Effect::SM_VIDEO},
  {"video_loop", Effect::SM_VIDEO_LOOP},
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

  timer_us_t tick = Timer::get_micros();
  YamlNodeArray yaml = YamlNodeArray::fromFile(filename, FFat);

  _version = atoi(yaml.getString("version", "0").c_str());

  if (_version != 1) {
    _error = "version mismatch";
    return false;
  }

  _type = yaml.isKeysExist("video") ? ST_VIDEO : ST_SEQUENCE;
  if (_type == ST_SEQUENCE) {
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
      DRAW_SEQUENCE ds;
      uint8_t result = parseDrawSequence(it.asString(), ds, _error);

      if (result == PD_SUCCESS) {
        _sequences.push_back(ds);
      }
      else if (result == PD_SKIP) {
        continue;
      }
      else {
        return false;
      }
    }

    auto boop_sequences = yaml.findKeys("boop_sequences");
    if (boop_sequences != nullptr && boop_sequences->isObject()) {
      for (const auto& it : boop_sequences->asObjects()) {
        DRAW_SEQUENCE ds;
        uint8_t result = parseDrawSequence(it.asString(), ds, _error);

        if (result == PD_SUCCESS) {
          _boop_sequences.push_back(ds);
        }
        else if (result == PD_SKIP) {
          continue;
        }
        else {
          return false;
        }
      }
    }
  }
  else { //if (_type == ST_VIDEO) {
    _video.path = yaml.getString("video:path", "");
    _video.start = yaml.getInt("video:start", 1);
    _video.end = yaml.getInt("video:end", 1);
    _video.fps = yaml.getInt("video:fps", 15);
    
    auto mode = modes.find(yaml.getString("video:mode", "mirror_only_offset"));
    if (mode == modes.end()) {
      _video.mode = Effect::DM_MIRROR_ONLY_OFFSET;
    }
    else {
      _video.mode = mode->second;
    }
    
    _video.loop = parse_bool(yaml.getString("video:loop", "true").c_str());

    _video.offset_x = yaml.getInt("video:offset_x", 0);
    _video.offset_y = yaml.getInt("video:offset_y", 0);
    _video.offset_mode = (yaml.getString("video:offset", "auto") == "auto") ? 1 : 0;
  }
  
  TF_LOGD(TAG, "yaml parsed (%lld us)", Timer::get_micros() - tick);

  return true;
}

uint8_t EffectParser::parseDrawSequence(const std::string& str, DRAW_SEQUENCE& ds, std::string& error) {
  auto split_result = my_split(str, ",");
  
  if (split_result.empty()) {
    return PD_SKIP;
  }

  auto method = methods.find(split_result[0].c_str());
  if (method == methods.end()) {
    error = "failed to parse [method] in [sequences]";
    return PD_FAIL;
  }
  ds.method = method->second;

  switch (ds.method) {
  case Effect::SM_NONE:
    if (split_result.size() < 2) {
      return PD_SKIP;
    }

    ds.display_time = (uint32_t)(atof(split_result[1].c_str()) * 1000);
    return PD_SUCCESS;
  case Effect::SM_DRAW:
  case Effect::SM_DRAW_90:
  case Effect::SM_DRAW_180:
  case Effect::SM_DRAW_270: {
    if (split_result.size() < 7) {
      error = "not enough arguments in [sequences]";
      return PD_FAIL;
    }

    ds.image = atoi(split_result[1].c_str());

    auto color = colors.find(split_result[2].c_str());
    if (color == colors.end()) {
      error = "failed to parse [color] in [sequences]";
      return PD_FAIL;
    }
    ds.color = color->second;

    auto mode = modes.find(split_result[3].c_str());
    if (mode == modes.end()) {
      error = "failed to parse [mode] in [sequences]";
      return PD_FAIL;
    }
    ds.mode = mode->second;

    ds.offset_x = atoi(split_result[4].c_str());
    ds.offset_y = atoi(split_result[5].c_str());
    ds.display_time = (uint32_t)(atof(split_result[6].c_str()) * 1000);
    return PD_SUCCESS;
  }
    break;
  case Effect::SM_VIDEO:
  case Effect::SM_VIDEO_LOOP: {
    if (split_result.size() < 5) {
      error = "not enough arguments in [sequences]";
      return PD_FAIL;
    }

    ds.image = atoi(split_result[1].c_str());

    auto mode = modes.find(split_result[2].c_str());
    if (mode == modes.end()) {
      error = "failed to parse [mode] in [sequences]";
      return PD_FAIL;
    }
    ds.mode = mode->second;

    ds.offset_x = atoi(split_result[3].c_str());
    ds.offset_y = atoi(split_result[4].c_str());
    return PD_SUCCESS;
  }
    break;
  }

  return PD_SKIP;
}

};
