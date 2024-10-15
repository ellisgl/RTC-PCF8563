#include <Arduino.h>
#include "rtc_date.h"

uint8_t RTC_Date::StringToUint8(const char *pString) {
    uint8_t value = 0;

    // skip leading 0 and spaces
    while ('0' == *pString || *pString == ' ') {
        ++pString;
    }

    // calculate number until we hit non-numeral char
    while ('0' <= *pString && *pString <= '9') {
        value *= 10;
        value += *pString - '0';
        ++pString;
    }

    return value;
}

RTC_Date::RTC_Date() : year(0), month(0), day(0), hour(0), minute(0), second(0) {
}

RTC_Date::RTC_Date(uint16_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t mm, uint8_t s)
    : year(y), month(m), day(d), hour(h), minute(mm), second(s) {
}

RTC_Date::RTC_Date(const char *date, const char *time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    year = 2000 + StringToUint8(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0]) {
        case 'J':
            if (date[1] == 'a') {
                month = 1;
                break;
            }

            if (date[2] == 'n') {
                month = 6;
                break;
            }

            month = 7;
            break;

        case 'F':
            month = 2;
            break;

        case 'A':
            month = date[1] == 'p' ? 4 : 8;
            break;

        case 'M':
            month = date[2] == 'r' ? 3 : 5;
            break;

        case 'S':
            month = 9;
            break;

        case 'O':
            month = 10;
            break;

        case 'N':
            month = 11;
            break;

        case 'D':
            month = 12;
            break;
    }

    day    = StringToUint8(date + 4);
    hour   = StringToUint8(time);
    minute = StringToUint8(time + 3);
    second = StringToUint8(time + 6);
}

bool RTC_Date::operator==(RTC_Date d) {
    return ((d.year == year) && (d.month == month) && (d.day == day) && (d.hour == hour) && (d.minute == minute));
}
