#pragma once

#include <stdint.h>

// FFat 파티션을 USB 저장소(MSC)로 노출하는 기능입니다.
// USB OTG가 있는 보드(ESP32-S3)에서 ARDUINO_USB_MODE=0, USE_USB_MSC 로 빌드해야 합니다.
//
// 동작 방식:
//   1. reboot(): RTC 메모리에 플래그를 남기고 재부팅
//   2. 부팅 시 플래그가 있으면 USB 저장소 모드로 진입 (isActive() == true)
//   3. run(): 다른 초기화 없이 FFat 파티션만 USB로 노출
//   4. PC에서 꺼내기 또는 USB 연결 해제 시 일반 모드로 재부팅
//
// Exposes the FFat partition as USB mass storage (MSC).
// Requires a board with USB OTG (ESP32-S3), built with ARDUINO_USB_MODE=0 and USE_USB_MSC.

namespace toaster {

namespace UsbStorage {

enum State : uint8_t {
  STATE_WAITING = 0,  // PC 연결 대기 / Waiting for the host
  STATE_CONNECTED,    // PC에 연결됨 / Connected to the host
  STATE_RESTARTING,   // 꺼내기 또는 연결 해제로 재부팅 중 / Restarting after eject or disconnect
};

typedef struct _Status {
  State state;
  bool error_boot;          // 치명적 오류로 진입했는지 / Entered because of a fatal error
  uint32_t capacity_kb;
  uint32_t read_bytes;
  uint32_t write_bytes;
  uint32_t last_read_ms;    // 0: 없음 / 0: none
  uint32_t last_write_ms;   // 0: 없음 / 0: none
} Status;

// run() 중 주기적으로 호출됩니다. (화면 표시용)
// Called periodically during run() (for display).
typedef void (*TickCallback)(const Status& status);

// USB 저장소 기능이 빌드에 포함되어 있는지 여부
// Whether the USB storage feature is built in.
bool isSupported();

// 이번 부팅이 USB 저장소 모드인지 여부
// Whether this boot is in USB storage mode.
bool isActive();

// 이번 부팅이 치명적 오류로 인한 USB 저장소 모드인지 여부
// Whether this boot entered USB storage mode because of a fatal error.
bool isErrorBoot();

// 다음 부팅을 USB 저장소 모드로 지정하고 재부팅합니다. (미지원 시 false 반환)
// Reboots into USB storage mode. Returns false if not supported.
bool reboot(bool on_error = false);

// FFat 파티션을 USB 저장소로 노출합니다. 꺼내기/연결 해제 시 재부팅하며 반환하지 않습니다.
// 파티션을 열 수 없으면 false를 반환합니다.
// Exposes the FFat partition. Reboots on eject/disconnect and never returns on success.
bool run(TickCallback on_tick = nullptr, uint32_t tick_interval_ms = 100);

}

};
