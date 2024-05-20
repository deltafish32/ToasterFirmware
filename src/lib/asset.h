#pragma once
#include "image.h"
#include "video.h"


namespace toaster {


class Asset {
public:
  Asset(const char* path, bool rgb565, uint32_t mjpeg_fps = 0);
  virtual ~Asset();

public:
  void release();
  bool isLoaded() {
    return (_type != ASSET_NONE);
  }

  uint8_t getType() const {
    return _type;
  }

  Image* getImage() {
    return (_type == ASSET_IMAGE) ? _image : nullptr;
  }

  const Image* getImage() const {
    return (_type == ASSET_IMAGE) ? _image : nullptr;
  }

  Video* getVideo() {
    return (_type == ASSET_VIDEO) ? _video : nullptr;
  }

  const Video* getVideo() const {
    return (_type == ASSET_VIDEO) ? _video : nullptr;
  }

  size_t getWidth() const {
    return (_type == ASSET_IMAGE) ? _image->getWidth() : 
           (_type == ASSET_VIDEO) ? _video->getWidth() : 0;
  }

  size_t getHeight() const {
    return (_type == ASSET_IMAGE) ? _image->getHeight() : 
           (_type == ASSET_VIDEO) ? _video->getHeight() : 0;
  }

public:
  enum ASSET_TYPE {
    ASSET_NONE = 0,
    ASSET_IMAGE,
    ASSET_VIDEO,
  };

protected:
  uint8_t _type{ASSET_NONE};

  union {
    void* _ptr{nullptr};
    Image* _image;
    Video* _video;
  };

};


};
