#include "effect_script.h"

#include "lib/logger.h"
#include "protogen.h"


namespace toaster {

static const char* TAG = "EffectScript";

static const int DYNAMIC_LOAD_MIN = 8;


EffectScript::EffectScript(const char* name) : FixedEffect(name) {
}


void EffectScript::init(Display& display) {
  Effect::init(display);
  
  if (_parser.getType() == ST_SEQUENCE) {
    // find only used images
    size_t image_count = _parser.getImages().size();
    size_t image_usage_count = 0;
    uint8_t* image_usage = new uint8_t[image_count];
    memset(image_usage, 0, sizeof(uint8_t) * image_count);

    for (const auto& it : _parser.getSequences()) {
      image_usage[it.image] = 1;
    }

    for (const auto& it : _parser.getBoopSequences()) {
      image_usage[it.image] = 1;
    }

    for (int i = 0; i < image_count; i++) {
      if (image_usage[i] != 0) {
        ++image_usage_count;
      }
    }

    timer_us_t tick = Timer::get_micros();

    size_t image_loaded = 0;
    for (int i = 0; i < image_count; i++) {
      auto image = (image_usage[i] != 0 && (image_usage_count < DYNAMIC_LOAD_MIN || image_loaded < DYNAMIC_LOAD_MIN)) ? display.load_png(_parser.getImages()[i].c_str()) : nullptr;
      
      if (image != nullptr) {
        ++image_loaded;
      }

      _images.push_back(image);
    }

    delete[] image_usage;

    _parser.releaseImages();
    
    TF_LOGD(TAG, "%s %d/%d images loaded (%lld us)", getName(), image_loaded, image_count, Timer::get_micros() - tick);
  }
  else { //if (_parser.getType() == ST_VIDEO) {
    auto& video = _parser.getVideoInfo();

    _video_offset_x = video.offset_x;
    _video_offset_y = video.offset_y;

    if (video.offset_mode != 0) {
      _parser.getVideoInfo().path.c_str();

      if (video.path.length() > 255) {
        TF_LOGW(TAG, "process: video path length exceededs 255 (%d, %s)", video.path.length(), video.path.c_str());
      }
      else {
        char s[256] = {0, };
        sprintf(s, video.path.c_str(), video.start);
        _images.push_back(display.load_png(s));

        int video_width = upng_get_width(_images.front());
        int video_height = upng_get_height(_images.front());

        _video_offset_x = (display.getWidth() / HUB75_PANEL_CHAIN) - video_width;
        _video_offset_y = (display.getHeight() - video_height) / 2;
      }
    }

    setStep(video.start);
  }

  _boop = false;

}


void EffectScript::process(Display& display) {
  if (_error) {
    return;
  }

  if (_parser.getType() == ST_SEQUENCE) {
    bool boop = Protogen._boopsensor.isBoop() && !_parser.getBoopSequences().empty();
    if (_staticMode == false) {
      if (_boop == false && boop) {
        _boop = true;
        setStep(0);
      }
      else if (_boop && boop == false) {
        _boop = false;
        setStep(0);
      }
    }

    const auto& sequence = (_boop == false) ? _parser.getSequences() : _parser.getBoopSequences();

    if (sequence.empty()) {
      TF_LOGE(TAG, "Sequence does not exist.");
      _error = true;
      return;
    }

    const auto& current = sequence[_step];

    switch (current.method) {
    case Effect::SM_NONE:
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270:
      if (current.image >= _images.size()) {
        TF_LOGE(TAG, "image index out of range. (%d / %d)", current.image, _images.size());
        _error = true;
        return;
      }

      if (_images[current.image] == nullptr) {
        size_t released = releaseSomeImages(display, current.image, sequence, 1);
        size_t loaded = loadNextImages(display, current.image, sequence, 1);
        TF_LOGW(TAG, "dynamic image loading, frame drops may occur (released: %d, loaded: %d)", released, loaded);
      }

      if (current.color == Effect::PC_ORIGINAL) {
        display.draw_png(_images[current.image], (DRAW_MODE)current.mode, current.offset_x, current.offset_y, current.method - Effect::SM_DRAW);
      }
      else {
        display.draw_png_newcolor(_images[current.image], _colorFunc, current.color, (DRAW_MODE)current.mode, current.offset_x, current.offset_y, current.method - Effect::SM_DRAW);
      }
      break;
    }

    if (sequence.size() >= 2 && timeout(current.display_time)) {
      if (_step >= sequence.size() - 1) {
        setStep(0);
      }
      else {
        setStep(_step + 1);
      }
    }
  }
  else { //if (_parser.getType() == ST_VIDEO) {
    const auto& video = _parser.getVideoInfo();

    if (_step >= video.start && _step <= video.end) {
      if (_recent_frame == nullptr) {
        if (video.path.length() > 255) {
          TF_LOGW(TAG, "process: video path length exceededs 255 (%d, %s)", video.path.length(), video.path.c_str());
        }
        else {
          char s[256] = {0, };
          sprintf(s, video.path.c_str(), _step);
          _recent_frame = display.load_png(s);
        }
      }

      display.draw_png(_recent_frame, (DRAW_MODE)video.mode, _video_offset_x, _video_offset_y, 0);

      if (_staticMode == false) {
        if (_step >= video.end - 1) {
          setStep(video.loop ? video.start : -1);

          if (video.loop == false) {
            TF_LOGI(TAG, "video end");
          }
        }
        else {
          setStep(_step + 1);
        }

        display.unload_png(_recent_frame);
        _recent_frame = nullptr;
      }
    }
  }
}


void EffectScript::release(Display& display) {
  if (_recent_frame != nullptr) {
    display.unload_png(_recent_frame);
    _recent_frame = nullptr;
  }

  for (auto& it : _images) {
    display.unload_png(it);
  }
  _images.clear();

  _parser.release();
  
  Effect::release(display);
}


bool EffectScript::loadScript(const char* name) {
  _script_name = name;

  char filename[256] = {0,};
  sprintf(filename, "/script/%s.yaml", name);

  bool result = _parser.open(filename);
  if (result == false) {
    TF_LOGE(TAG, "script %s parse failed (%s)", filename, _parser.getLastError());
  }

  // TF_LOGD(TAG, "script %s opened", filename);
  
  return result;
}


size_t EffectScript::releaseSomeImages(Display& display, int current_image, const std::vector<DRAW_SEQUENCE>& sequence, size_t to_release) {
  std::vector<int> image_list;

  for (int i = 0; i < _images.size(); i++) {
    if (_images[i] != nullptr) {
      image_list.push_back(i);
    }
  }

  auto release_func = [&](int i) {
    switch (sequence[i].method) {
    case Effect::SM_NONE:
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270:
      for (auto it = image_list.begin(); it != image_list.end(); ++it) {
        if (sequence[i].image == (*it)) {
          image_list.erase(it);
          break;
        }
      }
      break;
    }
  };

  for (int i = current_image + 1; image_list.size() > to_release && i < sequence.size(); i++) {
    release_func(i);
  }
  
  for (int i = 0; image_list.size() > to_release && i < current_image; i++) {
    release_func(i);
  }
  
  for (auto it : image_list) {
    display.unload_png(_images[it]);
    _images[it] = nullptr;
    
    // TF_LOGD(TAG, "releaseSomeImages: %s", _parser.getImages()[it].c_str());
  }
  
  return image_list.size();
}


size_t EffectScript::loadNextImages(Display& display, int current_image, const std::vector<DRAW_SEQUENCE>& sequence, size_t to_load) {
  size_t load_count = 0;

  std::vector<int> image_list;

  auto load_func = [&](int i) -> bool {
    switch (sequence[i].method) {
    case Effect::SM_NONE:
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270:
      if (_images[i] == nullptr) {
        _images[i] = display.load_png(_parser.getImages()[i].c_str());
        if (_images[i] == nullptr) {
          return false;
        }
        
        ++load_count;
        // TF_LOGD(TAG, "loadNextImages: %s", _parser.getImages()[i].c_str());
      }
      break;
    }

    return true;
  };

  for (int i = current_image; load_count < to_load && i < sequence.size(); i++) {
    if (!load_func(i)) {
      return load_count;
    }
  }
  
  for (int i = 0; load_count < to_load && i < current_image; i++) {
    if (!load_func(i)) {
      return load_count;
    }
  }
  
  return load_count;
}


};
