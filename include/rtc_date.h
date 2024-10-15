#pragma once

// Technically we shouldn't need ifndef/define/endif here, but it's just incase of any compiler oddness.
#ifndef RTC_DATE_H
#define RTC_DATE_H

#include <Arduino.h>

class RTC_Date
{
    public:
        RTC_Date();
        RTC_Date(const char *date, const char *time);
        RTC_Date(
            uint16_t year,
            uint8_t month,
            uint8_t day,
            uint8_t hour,
            uint8_t minute,
            uint8_t second
        );

        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;

        bool operator==(RTC_Date d);

    private:
        uint8_t StringToUint8(const char *pString);
};

#endif