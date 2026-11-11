#include "protogen.h"
#include "hal/usb_storage.h"


using namespace toaster;


void setup() {
  delay(1000);

  if (!Protogen.begin()) {
    // error
    if (!UsbStorage::isActive()) {
      UsbStorage::reboot(true);
    }

    while (1) { delay(1); }
  }
}


void loop() {
  Protogen.loop();
}
