#include <Arduino.h>
#include "rtc.h"
#include "lib/logger.h"


namespace toaster {


RTC::~RTC() {
  if (_type == RST_DS3231) {
    if (_ds3231 != nullptr) {
      delete _ds3231;
      _ds3231 = nullptr;
    }
  }
}


bool RTC::beginDS3231(bool nofail) {
  if (isBegin()) {
    return true;
  }

  _type = RST_DS3231;
  _ds3231 = new DS3231;
  _ds3231->begin();

  auto datetime = _ds3231->getDateTime();

  if (datetime.hour < 24) {
    sync();
  }
  else {
    TF_LOGW("RTC", "failed to connect.");
    
    if (!nofail) {
      delete _ds3231;
      _ds3231 = nullptr;
      return false;
    }
  }

  Worker::begin(2);

  return true;
}


bool RTC::work() {
  ++_blink;
  if (_blink >= 2) {
    _blink = 0;
    ++_second;
    if (_second >= 60) {
      _second = 0;
      ++_minute;
      if (_minute >= 60) {
        _minute = 0;
        ++_hour;
        if (_hour >= 24) {
          _hour = 0;
          ++_day;
          if (_day > getDaysInMonth(_year, _month)) {
            _day = 1;
            ++_month;
            if (_month > 12) {
              _month = 1;
              ++_year;
            }
          }
        }
      }
    }
  }

  return false;
}


void RTC::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  _year = year;
  _month = month;
  _day = day;
  _hour = hour;
  _minute = minute;
  _second = second;
  _blink = 0;

  if (_type == RST_DS3231) {
    if (_ds3231 != nullptr) {
      _ds3231->setDateTime(year, month, day, hour, minute, second);
    }
  }
}


bool RTC::sync() {
  if (_type == RST_DS3231 && _ds3231 != nullptr) {
    auto datetime = _ds3231->getDateTime();

    if (datetime.hour < 24) {
      _year = datetime.year;
      _month = datetime.month;
      _day = datetime.day;
      _hour = datetime.hour;
      _minute = datetime.minute;
      _second = datetime.second;
      _blink = 0;

      _synced = true;

      TF_LOGI("RTC", "datetime: %s", _ds3231->dateFormat("Y-m-d H:i:s", datetime));
      return true;
    }
  }

  return false;
}


bool RTC::isLeapYear(uint16_t year) {
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}


uint8_t RTC::getDaysInMonth(uint16_t year, uint8_t month) {
  static const uint8_t DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  return DAYS[month - 1] + ((month == 2 && isLeapYear(year)) ? 1 : 0);
}


};
