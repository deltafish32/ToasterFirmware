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
    initSequence(display);
  }
  else { //if (_parser.getType() == ST_VIDEO) {
    initVideo(display);
  }

  _boop = false;

}


void EffectScript::process(Display& display) {
  if (_error) {
    return;
  }

  _video_flag = false;
  
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

  if (_parser.getType() == ST_SEQUENCE) {
    const auto& sequence = (_boop == false) ? _parser.getSequences() : _parser.getBoopSequences();
    const auto& other_sequence = (_boop == false) ? _parser.getBoopSequences() : _parser.getSequences();

    if (sequence.empty()) {
      TF_LOGE(TAG, "Sequence does not exist.");
      _error = true;
      return;
    }

    processSequence(display, sequence, other_sequence);
  }
  else { //if (_parser.getType() == ST_VIDEO) {
    processVideo(display);
  }
}


void EffectScript::release(Display& display) {
  if (_video_legacy_frame != nullptr) {
    delete _video_legacy_frame;
    _video_legacy_frame = nullptr;
  }

  for (auto& it : _assets) {
    delete it;
  }
  _assets.clear();

  _parser.release();
  
  Effect::release(display);
}


bool EffectScript::loadScript(const char* name, const char* base_path) {
  _script_name = name;
  _base_path = base_path;

  char s[256] = {0, };
  sprintf(s, "script/%s.yaml", name);
  std::string filename = makeFileName(s);

  bool result = _parser.open(filename.c_str());
  if (result == false) {
    _base_path = Toaster::DEFAULT_BASE_PATH;
    filename = makeFileName(s);
    result = _parser.open(filename.c_str());
    if (result == false) {
      TF_LOGE(TAG, "script %s parse failed (%s)", filename.c_str(), _parser.getLastError());
    }
  }

  // TF_LOGD(TAG, "script %s opened", filename);
  
  return result;
}


std::string EffectScript::makeFileName(const char* filename) {
  std::string out_filename;

  if (filename[0] == '/') {
    out_filename = filename;
  }
  else {
    if (_base_path != "/") {
      out_filename = _base_path;
    }

    out_filename += '/';
    out_filename += filename;
  }

  return out_filename;
}


bool EffectScript::initSequence(Display& display) {
  // find only used images
  size_t image_count = _parser.getImages().size();
  size_t image_usage_count = 0;
  uint8_t* image_usage = new uint8_t[image_count];
  memset(image_usage, 0, sizeof(uint8_t) * image_count);

  for (const auto& it : _parser.getSequences()) {
    if (it.image < image_count) {
      image_usage[it.image] = 1;
    }
  }

  for (const auto& it : _parser.getBoopSequences()) {
    if (it.image < image_count) {
      image_usage[it.image] = 1;
    }
  }

  for (int i = 0; i < image_count; i++) {
    if (image_usage[i] != 0) {
      ++image_usage_count;
    }
  }

  timer_us_t tick = Timer::get_micros();

  size_t image_loaded = 0;
  for (int i = 0; i < image_count; i++) {
    std::string filename = makeFileName(_parser.getImages()[i].c_str());
    bool image_load = (image_usage[i] != 0 && (image_usage_count < DYNAMIC_LOAD_MIN || image_loaded < DYNAMIC_LOAD_MIN));
    auto asset = image_load ? new Asset(filename.c_str(), Protogen.isRGB565()) : nullptr;
    
    if (asset != nullptr) {
      ++image_loaded;
    }

    _assets.push_back(asset);
  }

  delete[] image_usage;

  TF_LOGD(TAG, "%s %d/%d images loaded (%lld us)", getName(), image_loaded, image_count, Timer::get_micros() - tick);
  return true;
}


bool EffectScript::initVideo(Display& display) {
  auto& video = _parser.getVideoInfo();

  _video_legacy = true;
  _video_index = 0;
  
  const char* ext = strrchr(video.path.c_str(), '.');
  if (ext != nullptr) {
    ++ext;
    
    if (strcasecmp(ext, "png") == 0 || strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
      if (video.path.length() > 255) {
        TF_LOGE(TAG, "initVideo: video path length exceededs 255 (%d, %s)", video.path.length(), video.path.c_str());
        _error = true;
        return false;
      }
      
      if (_video_legacy_frame != nullptr) {
        delete _video_legacy_frame;
      }

      char s[256] = {0, };
      sprintf(s, video.path.c_str(), _step);
      std::string filename = makeFileName(s);

      _video_legacy_frame = new Asset(filename.c_str(), Protogen.isRGB565());
      if (_video_legacy_frame == nullptr || !_video_legacy_frame->isLoaded()) {
        TF_LOGE(TAG, "initVideo: video first frame load failed. (%s)", video.path.c_str());
        _error = true;
        if (_video_legacy_frame != nullptr) {
          delete _video_legacy_frame;
          _video_legacy_frame = nullptr;
        }
        return false;
      }
    }
    else { // gif, mjpeg
      std::string filename = makeFileName(video.path.c_str());
      auto asset = new Asset(filename.c_str(), Protogen.isRGB565(), _parser.getVideoInfo().fps);
      if (asset != nullptr) {
        if (!asset->isLoaded()) {
          delete asset;
        }
        else {
          _assets.push_back(asset);

          _video_legacy = false;
        }
      }
    }
  }

  _video_offset_x = video.offset_x;
  _video_offset_y = video.offset_y;

  if (video.offset_mode != 0) {
    if (video.path.length() > 255) {
      TF_LOGW(TAG, "process: video path length exceededs 255 (%d, %s)", video.path.length(), video.path.c_str());
    }
    else {
      if (_assets.empty()) {
        TF_LOGE(TAG, "asset load failed.");
        _error = true;
        return false;
      }

      Asset* asset;
      
      if (_video_legacy) {
        char s[256] = {0, };
        sprintf(s, video.path.c_str(), video.start);
        std::string filename = makeFileName(s);

        _assets.push_back(new Asset(filename.c_str(), Protogen.isRGB565()));

        asset = _assets.front();
        if (asset == nullptr || asset->getType() != Asset::ASSET_IMAGE) {
          TF_LOGE(TAG, "asset is not image.");
          _error = true;
          return false;
        }
      }
      else {
        asset = _assets.front();
        if (asset == nullptr || asset->getType() != Asset::ASSET_VIDEO) {
          TF_LOGE(TAG, "asset is not video.");
          _error = true;
          return false;
        }
      }

      int video_width = asset->getWidth();
      int video_height = asset->getHeight();

      _video_offset_x = (display.getWidth() / HUB75_PANEL_CHAIN) - video_width;
      _video_offset_y = (display.getHeight() - video_height) / 2;
    }
  }

  setStep(video.start);

  return true;
}


void EffectScript::processSequence(Display& display, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence) {
  const auto& current = sequence[_step];

  if (current.method != Effect::SM_NONE) {
    if (current.image >= _assets.size()) {
      TF_LOGE(TAG, "processSequence: asset index out of range. (%d / %d)", current.image, _assets.size());
      _error = true;
      return;
    }

    if (_assets[current.image] == nullptr) {
      size_t released = releaseSomeImages(display, _step, sequence, other_sequence, 1);
      size_t loaded = loadNextImages(display, _step, sequence, other_sequence, std::max((size_t)1, released));
      TF_LOGW(TAG, "processSequence: dynamic image loading, frame drops may occur (released: %d, loaded: %d)", released, loaded);
    }

    if (_assets[current.image] == nullptr) {
      TF_LOGE(TAG, "processSequence: asset load failed. (asset: %d)", current.image);
      _error = true;
      return;
    }
  }

  switch (current.method) {
  case Effect::SM_NONE:
    if (sequence.size() >= 2 && timeout(current.display_time)) {
      setStep((_step < sequence.size() - 1) ? _step + 1 : 0);
    }
    break;
  case Effect::SM_DRAW:
  case Effect::SM_DRAW_90:
  case Effect::SM_DRAW_180:
  case Effect::SM_DRAW_270: {
    if (_assets[current.image]->getType() != Asset::ASSET_IMAGE) {
      TF_LOGE(TAG, "processSequence: asset type is not image. (asset: %d, type: %d)", current.image, _assets[current.image]->getType());
      _error = true;
      return;
    }

    const auto& image = _assets[current.image]->getImage();

    if (current.color == Effect::PC_ORIGINAL) {
      display.draw_image(image, (DRAW_MODE)current.mode, current.offset_x, current.offset_y, current.method - Effect::SM_DRAW);
    }
    else {
      display.draw_image_newcolor(image, _colorFunc, current.color, (DRAW_MODE)current.mode, current.offset_x, current.offset_y, current.method - Effect::SM_DRAW);
    }

    if (sequence.size() >= 2 && timeout(current.display_time)) {
      setStep((_step < sequence.size() - 1) ? _step + 1 : 0);
    }
  }
    break;
  case Effect::SM_VIDEO:
  case Effect::SM_VIDEO_LOOP: {
    if (_assets[current.image]->getType() != Asset::ASSET_VIDEO) {
      TF_LOGE(TAG, "processSequence: asset type is not video. (asset: %d, type: %d)", current.image, _assets[current.image]->getType());
      _error = true;
      return;
    }

    const auto& video = _assets[current.image]->getVideo();

    display.draw_image(video->getImage(), (DRAW_MODE)current.mode, current.offset_x, current.offset_y, 0);

    _video_flag = true;
    _video_index = current.image;

    if (Protogen.isAdaptiveFps() || timeout(video->getPF_ms())) {
      if (current.method == Effect::SM_VIDEO && sequence.size() >= 2) {
        // no loop
        if (!video->nextFrame(false)) {
          setStep((_step < sequence.size() - 1) ? _step + 1 : 0);
        }
      }
      else {
        // loop
        video->nextFrame(true);

        restartTimer();
      }
    }
  }
    break;
  }
}


void EffectScript::processVideo(Display& display) {
  const auto& video_info = _parser.getVideoInfo();

  if (_video_legacy) {
    if (_step >= video_info.start && _step <= video_info.end) {
      if (_video_legacy_frame == nullptr) {
        if (video_info.path.length() > 255) {
          TF_LOGE(TAG, "processVideo: video path length exceededs 255 (%d, %s)", video_info.path.length(), video_info.path.c_str());
          _error = true;
          return;
        }
        
        char s[256] = {0, };
        sprintf(s, video_info.path.c_str(), _step);
        std::string filename = makeFileName(s);

        _video_legacy_frame = new Asset(filename.c_str(), Protogen.isRGB565());
        if (_video_legacy_frame == nullptr || !_video_legacy_frame->isLoaded()) {
          TF_LOGE(TAG, "processVideo: video frame load failed. (%s)", video_info.path.c_str());
          if (_video_legacy_frame != nullptr) {
            delete _video_legacy_frame;
            _video_legacy_frame = nullptr;
          }
          _error = true;
          return;
        }
      }

      if (_video_legacy_frame->getType() != Asset::ASSET_IMAGE) {
        TF_LOGE(TAG, "processVideo: asset type is not image. (asset: %d, type: %d)", _step, _video_legacy_frame->getType());
        _error = true;
        return;
      }

      display.draw_image(_video_legacy_frame->getImage(), (DRAW_MODE)video_info.mode, _video_offset_x, _video_offset_y, 0);

      if (_staticMode == false) {
        if (_step >= video_info.end - 1) {
          setStep(video_info.loop ? video_info.start : -1);

          if (video_info.loop == false) {
            TF_LOGI(TAG, "video end");
          }
        }
        else {
          setStep(_step + 1);
        }

        if (_video_legacy_frame != nullptr) {
          delete _video_legacy_frame;
          _video_legacy_frame = nullptr;
        }
      }
    }
  }
  else {
    if (_assets.empty()) {
      TF_LOGE(TAG, "processVideo: asset does not exist.");
      _error = true;
      return;
    }

    auto asset = _assets.front();
    if (asset->getType() != Asset::ASSET_VIDEO) {
      TF_LOGE(TAG, "processVideo: asset type is not video. (asset: %d, type: %d)", _step, asset->getType());
      _error = true;
      return;
    }

    display.draw_image(asset->getVideo()->getImage(), (DRAW_MODE)video_info.mode, _video_offset_x, _video_offset_y, 0);

    if (_staticMode == false) {
      if (Protogen.isAdaptiveFps() || timeout(asset->getVideo()->getPF_ms())) {
        asset->getVideo()->nextFrame(true);
      }
    }
  }
}


size_t EffectScript::releaseSomeImages(Display& display, int current_sequence, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence, size_t to_release) {
  std::vector<int> image_list;

  for (int i = 0; i < _assets.size(); i++) {
    if (_assets[i] != nullptr) {
      image_list.push_back(i);
    }
  }

  auto release_func = [&image_list](const std::vector<DRAW_SEQUENCE>& seq, int i) {
    switch (seq[i].method) {
    case Effect::SM_NONE:
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270: {
      int image_index = seq[i].image;
      for (auto it = image_list.begin(); it != image_list.end(); ++it) {
        if (image_index == (*it)) {
          image_list.erase(it);
          break;
        }
      }
    }
      break;
    }
  };

  for (int i = current_sequence + 1; image_list.size() > to_release && i < sequence.size(); i++) {
    release_func(sequence, i);
  }
  
  for (int i = 0; image_list.size() > to_release && i < current_sequence; i++) {
    release_func(sequence, i);
  }
  
  for (int i = 0; image_list.size() > to_release && i < other_sequence.size(); i++) {
    release_func(other_sequence, i);
  }
  
  for (auto it : image_list) {
    delete _assets[it];
    _assets[it] = nullptr;
    
    // TF_LOGD(TAG, "releaseSomeImages: %s", _parser.getImages()[it].c_str());
  }
  
  return image_list.size();
}


size_t EffectScript::loadNextImages(Display& display, int current_sequence, const std::vector<DRAW_SEQUENCE>& sequence, const std::vector<DRAW_SEQUENCE>& other_sequence, size_t to_load) {
  size_t load_count = 0;

  std::vector<int> image_list;

  auto load_func = [&](const std::vector<DRAW_SEQUENCE>& seq, int i) -> bool {
    switch (seq[i].method) {
    case Effect::SM_NONE:
      break;
    case Effect::SM_DRAW:
    case Effect::SM_DRAW_90:
    case Effect::SM_DRAW_180:
    case Effect::SM_DRAW_270: {
      int image_index = seq[i].image;
      if (_assets[image_index] == nullptr) {
        std::string filename = makeFileName(_parser.getImages()[image_index].c_str());
        _assets[image_index] = new Asset(filename.c_str(), Protogen.isRGB565());
        if (_assets[image_index] == nullptr) {
          TF_LOGW(TAG, "loadNextImages failed (%d, %s)", image_index, _parser.getImages()[image_index].c_str());
          return false;
        }
        
        ++load_count;
      }
    }
      break;
    }

    return true;
  };

  for (int i = current_sequence; load_count < to_load && i < sequence.size(); i++) {
    if (!load_func(sequence, i)) {
      return load_count;
    }
  }
  
  for (int i = 0; load_count < to_load && i < current_sequence; i++) {
    if (!load_func(sequence, i)) {
      return load_count;
    }
  }
  
  for (int i = 0; load_count < to_load && i < other_sequence.size(); i++) {
    if (!load_func(other_sequence, i)) {
      return load_count;
    }
  }
  
  return load_count;
}


};
