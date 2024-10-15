#pragma once

// Technically we shouldn't need ifndef/define/endif here, but it's just incase of any compiler oddness.
#ifndef RTC_ALARM_H
#define RTC_ALARM_H

#include <Arduino.h>

class RTC_Alarm
{
    public:
        RTC_Alarm(void);
        RTC_Alarm(
            uint8_t minute,
            uint8_t hour,
            uint8_t day,
            uint8_t weekday
        );

        uint8_t minute;
        uint8_t hour;
        uint8_t day;
        uint8_t weekday;
};

#endif