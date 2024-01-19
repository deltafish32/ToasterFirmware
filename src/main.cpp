#include "protogen.h"


using namespace toaster;


void setup() {
  delay(1000);

  if (!Protogen.begin()) {
    // error
    while (1) { delay(1); }
  }
}


void loop() {
  Protogen.loop();
}
