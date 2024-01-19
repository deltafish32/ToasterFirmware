#pragma once


#define TF_LOG(TAG, STR...)    { Serial.printf("%s: ", TAG); Serial.printf(STR); Serial.println(); }
