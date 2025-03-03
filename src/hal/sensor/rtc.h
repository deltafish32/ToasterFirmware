#pragma once
#include "lib/worker.h"
#include "DS3231.h"


namespace toaster {

class RTC : public Worker {
public:
  RTC() {}
  virtual ~RTC();

private:
  bool begin() { return false; }

public:
  bool beginDS3231(bool nofail);
  virtual bool work();

  bool isBegin() const {
    return (_ptr != nullptr);
  }

  bool isSynced() const {
    return _synced;
  }

  uint16_t getYear() const {
    return _year;
  }

  uint8_t getMonth() const {
    return _month;
  }

  uint8_t getDay() const {
    return _day;
  }

  uint8_t getHour() const {
    return _hour;
  }

  uint8_t getMinute() const {
    return _minute;
  }

  uint8_t getSecond() const {
    return _second;
  }

  uint8_t getBlink() const {
    return _blink;
  }

	void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
  bool sync();
  
  static bool isLeapYear(uint16_t year);
  static uint8_t getDaysInMonth(uint16_t year, uint8_t month);

protected:
  enum {
    RST_NONE = 0,
    RST_DS3231,
  };

protected:
  uint8_t _type{RST_NONE};
  bool _synced{false};
  uint16_t _year{0};
  uint8_t _month{0};
  uint8_t _day{0};
  uint8_t _hour{0};
  uint8_t _minute{0};
  uint8_t _second{0};
  uint8_t _blink{0};

  union {
    void* _ptr{nullptr};
    DS3231* _ds3231;
  };

};

};
