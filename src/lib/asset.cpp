#include "config/configure.h"
#include "asset.h"
#include "logger.h"


namespace toaster {

static const char* TAG = "Asset";


Asset::Asset(const char* path, bool rgb565, uint32_t mjpeg_fps) {
  const char* ext = strrchr(path, '.');
  if (ext == nullptr) {
    return;
  }

  uint8_t image_type = Image::IMAGE_NONE;
  uint8_t video_type = Video::VIDEO_NONE;

  if (strcasecmp(ext + 1, "png") == 0) {
    image_type = Image::IMAGE_PNG;
  }

  if (strcasecmp(ext + 1, "jpg") == 0 || strcasecmp(ext + 1, "jpeg") == 0) {
    image_type = Image::IMAGE_JPEG;
  }

  if (strcasecmp(ext + 1, "gif") == 0) {
    video_type = Video::VIDEO_GIF;
  }

  if (strcasecmp(ext + 1, "mjpeg") == 0) {
    video_type = Video::VIDEO_MJPEG;
  }

  if (image_type != Image::IMAGE_NONE) {
    _type = ASSET_IMAGE;
    _image = new Image(path, rgb565);
  }
  else if (video_type != Video::VIDEO_NONE) {
    _type = ASSET_VIDEO;
    _video = new Video(path, mjpeg_fps);
  }
}


Asset::~Asset() {
  release();
}

void Asset::release() {
  if (_type == ASSET_IMAGE) {
    if (_image != nullptr) {
      delete _image;
      _image = nullptr;
    }
  }
  else if (_type == ASSET_VIDEO) {
    if (_video != nullptr) {
      delete _video;
      _video = nullptr;
    }
  }
}


};
