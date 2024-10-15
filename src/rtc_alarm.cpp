#include <Arduino.h>
#include "rtc_alarm.h"

RTC_Alarm::RTC_Alarm(uint8_t m, uint8_t h, uint8_t d, uint8_t w) : minute(m), hour(h), day(d), weekday(w) {
}