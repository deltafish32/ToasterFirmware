#include "usb_storage.h"

#include <Arduino.h>


#ifdef USE_USB_MSC

#if ARDUINO_USB_MODE
#error "USE_USB_MSC requires ARDUINO_USB_MODE=0 (TinyUSB)."
#endif

#include <USB.h>
#include <USBMSC.h>
#include <esp32-hal-tinyusb.h>
#include <esp_attr.h>
#include <esp_partition.h>
#include <wear_levelling.h>

#ifdef USE_SD
#include <SPI.h>
#include "sd_diskio.h"
#include "ff.h"
#include "diskio.h"
#endif

#include "lib/logger.h"


namespace toaster {

namespace UsbStorage {

static const char* TAG = "UsbStorage";

static const char* FFAT_PARTITION_LABEL = "ffat";

// suspend가 이 시간 이상 유지되면 연결 해제로 판단 (배터리 사용 중 USB 분리, PC 절전)
// Treat as disconnected when suspended for this long (USB unplugged on battery, host sleep).
static const uint32_t SUSPEND_DEBOUNCE_MS = 300;

// 미연결(unmounted) 상태가 이 시간 이상 유지되면 연결 해제로 판단
// 호스트가 오류 복구를 위해 버스 리셋을 하면 잠시 미연결 상태가 되므로, 짧은 미연결은 무시합니다.
// Treat as disconnected when unmounted for this long.
// A bus reset by the host (error recovery) unmounts briefly, so short periods are ignored.
static const uint32_t UNMOUNT_DEBOUNCE_MS = 3000;

// 꺼내기 응답이 호스트에 전달될 시간
// Time for the eject response to reach the host.
static const uint32_t RESTART_DELAY_MS = 200;

// 소프트웨어 리셋에서만 유지되는 RTC 메모리 (전원을 끄면 사라짐)
// RTC memory retained across software resets only.
// 하위 8비트는 플래그
// The low 8 bits hold flags.
static const uint32_t BOOT_MAGIC = 0x55534D00; // "USM"
static const uint32_t BOOT_MAGIC_MASK = 0xFFFFFF00;
static const uint32_t BOOT_FLAG_ERROR = 0x01;
static const uint32_t BOOT_FLAG_SD = 0x02;
RTC_NOINIT_ATTR static uint32_t s_boot_magic;
RTC_NOINIT_ATTR static uint32_t s_boot_magic_inv;

static bool s_active = false;
static bool s_error_boot = false;
static bool s_sd_present = false;
static USBMSC* s_msc = nullptr;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static size_t s_sector_size = 0;
static uint8_t* s_sector_buffer = nullptr;
static volatile bool s_eject_requested = false;

#ifdef USE_SD
// 4MHz에서는 64KB에 약 200ms가 걸려 USB 전체가 그동안 멈춥니다. (카드 초기화는 400kHz로 진행)
// At 4MHz a 64KB transfer takes ~200ms and stalls the whole USB device. (Card init runs at 400kHz.)
static const uint32_t SD_FREQUENCY = 20000000;
static const uint32_t SD_SECTOR_SIZE = 512;
static USBMSC* s_sd_msc = nullptr;
static uint8_t s_sd_pdrv = 0xFF;
static volatile bool s_sd_eject_requested = false;
#endif

static volatile uint64_t s_read_bytes = 0;
static volatile uint64_t s_write_bytes = 0;
static volatile uint32_t s_last_read_ms = 0;
static volatile uint32_t s_last_write_ms = 0;
static volatile uint32_t s_ffat_last_write_ms = 0;
static volatile uint32_t s_sd_last_write_ms = 0;

// 한 드라이브를 꺼낸 뒤, 다른 드라이브에 이 시간 동안 쓰기가 없으면 재부팅합니다. (복사 중 재부팅 방지)
// After one drive is ejected, reboot once the other drive has had no writes for this long (avoids rebooting mid-copy).
static const uint32_t EJECT_IDLE_MS = 2000;


// MSC 인터페이스는 USB.begin() 전에 등록되어야 하므로 전역 생성자에서 처리합니다.
// (ARDUINO_USB_CDC_ON_BOOT 사용 시 USB.begin()은 setup() 이전에 호출됨)
// The MSC interface must be registered before USB.begin(), so it is done in a global constructor.
class BootCheck {
public:
  BootCheck() {
    if ((s_boot_magic & BOOT_MAGIC_MASK) == BOOT_MAGIC && s_boot_magic_inv == ~s_boot_magic) {
      s_active = true;
      s_error_boot = (s_boot_magic & BOOT_FLAG_ERROR) != 0;
      s_msc = new USBMSC();
#ifdef USE_SD
      // 호스트가 연결 직후 LUN 개수를 읽으므로 여기서 정합니다.
      // The host reads the LUN count right after connecting, so it is fixed here.
      if (s_boot_magic & BOOT_FLAG_SD) {
        s_sd_msc = new USBMSC();
      }
#endif
    }

    // 한 번만 진입하도록 즉시 해제
    // Clear immediately so that it is entered only once.
    s_boot_magic = 0;
    s_boot_magic_inv = 0;
  }
};

static BootCheck s_boot_check;


static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  size_t addr = (size_t)lba * s_sector_size + offset;

  if (wl_read(s_wl_handle, addr, buffer, bufsize) != ESP_OK) {
    return -1;
  }

  s_read_bytes += bufsize;
  s_last_read_ms = millis() | 1;

  return bufsize;
}


static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  size_t addr = (size_t)lba * s_sector_size + offset;
  uint32_t done = 0;

  while (done < bufsize) {
    size_t cur = addr + done;
    size_t sector_start = cur - (cur % s_sector_size);
    size_t in_offset = cur - sector_start;
    size_t len = std::min((size_t)(s_sector_size - in_offset), (size_t)(bufsize - done));
    const uint8_t* src = buffer + done;

    // 섹터 일부만 쓰는 경우 read-modify-write
    // Read-modify-write for partial sector writes.
    if (in_offset != 0 || len != s_sector_size) {
      if (wl_read(s_wl_handle, sector_start, s_sector_buffer, s_sector_size) != ESP_OK) {
        return -1;
      }
      memcpy(s_sector_buffer + in_offset, src, len);
      src = s_sector_buffer;
    }

    if (wl_erase_range(s_wl_handle, sector_start, s_sector_size) != ESP_OK) {
      return -1;
    }

    if (wl_write(s_wl_handle, sector_start, src, s_sector_size) != ESP_OK) {
      return -1;
    }

    done += len;
  }

  s_write_bytes += bufsize;
  s_last_write_ms = millis() | 1;
  s_ffat_last_write_ms = s_last_write_ms;

  return bufsize;
}


static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  if (load_eject && !start) {
    s_eject_requested = true;
  }

  return true;
}


#ifdef USE_SD
static int32_t onSdRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  if (offset != 0 || bufsize % SD_SECTOR_SIZE != 0) {
    return -1;
  }

  if (ff_disk_read(s_sd_pdrv, (BYTE*)buffer, lba, bufsize / SD_SECTOR_SIZE) != RES_OK) {
    return -1;
  }

  s_read_bytes += bufsize;
  s_last_read_ms = millis() | 1;

  return bufsize;
}


static int32_t onSdWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  if (offset != 0 || bufsize % SD_SECTOR_SIZE != 0) {
    return -1;
  }

  if (ff_disk_write(s_sd_pdrv, buffer, lba, bufsize / SD_SECTOR_SIZE) != RES_OK) {
    return -1;
  }

  s_write_bytes += bufsize;
  s_last_write_ms = millis() | 1;
  s_sd_last_write_ms = s_last_write_ms;

  return bufsize;
}


static bool onSdStartStop(uint8_t power_condition, bool start, bool load_eject) {
  if (load_eject && !start) {
    s_sd_eject_requested = true;
  }

  return true;
}


// 카드를 초기화만 하고 마운트하지 않습니다. (PC와 동시에 파일 시스템을 쓰지 않도록)
// Initializes the card without mounting it, so the file system is not used together with the host.
static bool beginSd(uint32_t& sector_count) {
  SPI.begin();

  s_sd_pdrv = sdcard_init(PIN_FSPI_CS, &SPI, SD_FREQUENCY);
  if (s_sd_pdrv == 0xFF) {
    return false;
  }

  if ((ff_disk_initialize(s_sd_pdrv) & STA_NOINIT) != 0 || sdcard_sector_size(s_sd_pdrv) != SD_SECTOR_SIZE) {
    sdcard_uninit(s_sd_pdrv);
    s_sd_pdrv = 0xFF;
    return false;
  }

  sector_count = sdcard_num_sectors(s_sd_pdrv);

  return sector_count > 0;
}
#endif


bool isSupported() {
  return true;
}


void setSdCardPresent(bool present) {
  s_sd_present = present;
}


bool isActive() {
  return s_active;
}


bool isErrorBoot() {
  return s_error_boot;
}


bool reboot(bool on_error) {
  TF_LOGI(TAG, "reboot into USB storage mode%s%s", on_error ? " (error)" : "", s_sd_present ? " with SD card" : "");

  s_boot_magic = BOOT_MAGIC | (on_error ? BOOT_FLAG_ERROR : 0) | (s_sd_present ? BOOT_FLAG_SD : 0);
  s_boot_magic_inv = ~s_boot_magic;

  Serial.flush();
  delay(100);

  ESP.restart();

  return true;
}


bool run(TickCallback on_tick, uint32_t tick_interval_ms) {
  if (!s_active || s_msc == nullptr) {
    return false;
  }

  const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, FFAT_PARTITION_LABEL);
  if (partition == nullptr) {
    TF_LOGE(TAG, "run: FFat partition not found");
    return false;
  }

  esp_err_t err = wl_mount(partition, &s_wl_handle);
  if (err != ESP_OK) {
    TF_LOGE(TAG, "run: wl_mount failed (0x%x)", err);
    s_wl_handle = WL_INVALID_HANDLE;
    return false;
  }

  s_sector_size = wl_sector_size(s_wl_handle);
  uint32_t sector_count = wl_size(s_wl_handle) / s_sector_size;

  s_sector_buffer = (uint8_t*)malloc(s_sector_size);
  if (s_sector_buffer == nullptr) {
    TF_LOGE(TAG, "run: memory allocation failed");
    wl_unmount(s_wl_handle);
    s_wl_handle = WL_INVALID_HANDLE;
    return false;
  }

  s_msc->vendorID("Toaster");
  s_msc->productID("FFat");
  s_msc->productRevision("1.0");
  s_msc->onStartStop(onStartStop);
  s_msc->onRead(onRead);
  s_msc->onWrite(onWrite);

  if (!s_msc->begin(sector_count, s_sector_size)) {
    TF_LOGE(TAG, "run: MSC begin failed");
    wl_unmount(s_wl_handle);
    s_wl_handle = WL_INVALID_HANDLE;
    return false;
  }

  s_msc->mediaPresent(true);

  TF_LOGI(TAG, "USB storage mode (sector: %d, count: %d, size: %d KB)", (int)s_sector_size, (int)sector_count, (int)(sector_count * s_sector_size / 1024));

  Status status = {};
  status.state = STATE_WAITING;
  status.error_boot = s_error_boot;
  status.capacity_kb = sector_count * s_sector_size / 1024;

  bool sd_active = false;
#ifdef USE_SD
  if (s_sd_msc != nullptr) {
    uint32_t sd_sector_count = 0;

    s_sd_msc->vendorID("Toaster");
    s_sd_msc->productID("SD");
    s_sd_msc->productRevision("1.0");
    s_sd_msc->onStartStop(onSdStartStop);
    s_sd_msc->onRead(onSdRead);
    s_sd_msc->onWrite(onSdWrite);

    if (beginSd(sd_sector_count) && s_sd_msc->begin(sd_sector_count, SD_SECTOR_SIZE)) {
      s_sd_msc->mediaPresent(true);
      sd_active = true;
      status.capacity_kb += sd_sector_count / (1024 / SD_SECTOR_SIZE);

      TF_LOGI(TAG, "SD card (count: %u, size: %u MB)", (unsigned)sd_sector_count, (unsigned)(sd_sector_count / (1024 * 1024 / SD_SECTOR_SIZE)));
    }
    else {
      status.sd_error = true;
      TF_LOGE(TAG, "run: SD card initialization failed");
    }
  }
#endif

  bool ffat_ejected = false;

  auto tick = [&]() {
    if (on_tick != nullptr) {
      status.read_bytes = s_read_bytes;
      status.write_bytes = s_write_bytes;
      status.last_read_ms = s_last_read_ms;
      status.last_write_ms = s_last_write_ms;
      on_tick(status);
    }
  };

  uint32_t suspend_start = 0;
  uint32_t unmount_start = 0;
  uint32_t last_tick = millis();

  tick();

  while (1) {
    uint32_t now = millis();

    // 드라이브 하나를 꺼내면, 남은 드라이브에 쓰기가 멈춘 뒤 재부팅합니다.
    // When one drive is ejected, reboot once writes to the remaining drive have stopped.
    bool sd_ejected = false;
    if (s_eject_requested && !ffat_ejected) {
      ffat_ejected = true;
      s_msc->mediaPresent(false);
      TF_LOGI(TAG, "FFat ejected by host");
    }
#ifdef USE_SD
    if (s_sd_eject_requested && sd_active) {
      sd_active = false;
      s_sd_msc->mediaPresent(false);
      TF_LOGI(TAG, "SD card ejected by host");
    }
    sd_ejected = s_sd_eject_requested && s_sd_msc != nullptr;
#endif
    if (ffat_ejected || sd_ejected) {
      uint32_t other_last_write = 0;
      if (!ffat_ejected) {
        other_last_write = s_ffat_last_write_ms;
      }
      else if (sd_active) {
        other_last_write = s_sd_last_write_ms;
      }

      // 쓰기는 USB 태스크에서 기록되므로 now보다 나중일 수 있어, 부호 있는 차이로 비교합니다.
      // Writes are recorded by the USB task and can be newer than now, so compare as a signed difference.
      if (other_last_write == 0 || (int32_t)(millis() - other_last_write) >= (int32_t)EJECT_IDLE_MS) {
        TF_LOGI(TAG, "ejected by host");
        break;
      }

      if (status.state != STATE_FINISHING) {
        status.state = STATE_FINISHING;
        TF_LOGI(TAG, "waiting for writes to the other drive to finish");
      }
    }

    bool mounted = tud_mounted();
    bool suspended = tud_suspended();

    if (status.state == STATE_WAITING) {
      if (mounted && !suspended) {
        status.state = STATE_CONNECTED;
        TF_LOGI(TAG, "host connected");
      }
    }
    else {
      // suspend: USB 분리(배터리 사용 중) 또는 PC 절전
      // suspend: USB unplugged (on battery) or host sleep
      if (suspended) {
        if (suspend_start == 0) {
          suspend_start = now | 1;
        }
        else if (now - suspend_start >= SUSPEND_DEBOUNCE_MS) {
          TF_LOGI(TAG, "host disconnected (suspended)");
          break;
        }
      }
      else {
        suspend_start = 0;
      }

      // 미연결: 버스 리셋 중에는 잠시 발생하므로 오래 지속될 때만 연결 해제로 판단
      // Unmounted: happens briefly during a bus reset, so only a long period counts as disconnected.
      if (!mounted) {
        if (unmount_start == 0) {
          unmount_start = now | 1;
          TF_LOGD(TAG, "host unmounted (bus reset?)");
        }
        else if (now - unmount_start >= UNMOUNT_DEBOUNCE_MS) {
          TF_LOGI(TAG, "host disconnected (unmounted)");
          break;
        }
      }
      else {
        if (unmount_start != 0) {
          TF_LOGD(TAG, "host mounted again (%d ms)", (int)(now - unmount_start));
        }
        unmount_start = 0;
      }
    }

    if (now - last_tick >= tick_interval_ms) {
      last_tick = now;
      tick();
    }

    delay(10);
  }

  s_msc->mediaPresent(false);
#ifdef USE_SD
  if (s_sd_msc != nullptr) {
    s_sd_msc->mediaPresent(false);
  }
#endif

  status.state = STATE_RESTARTING;
  tick();

  delay(RESTART_DELAY_MS);

  wl_unmount(s_wl_handle);
  s_wl_handle = WL_INVALID_HANDLE;

  TF_LOGI(TAG, "restart");
  Serial.flush();

  ESP.restart();

  return true;
}

}

};


#else // USE_USB_MSC


namespace toaster {

namespace UsbStorage {

bool isSupported() {
  return false;
}

bool isActive() {
  return false;
}

bool isErrorBoot() {
  return false;
}

void setSdCardPresent(bool present) {
}

bool reboot(bool on_error) {
  return false;
}

bool run(TickCallback on_tick, uint32_t tick_interval_ms) {
  return false;
}

}

};


#endif // USE_USB_MSC
