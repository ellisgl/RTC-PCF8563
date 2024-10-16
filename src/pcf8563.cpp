/**
 * pcf8563.cpp - Arduino library for NXP PCF8563 RTC chip.
 * Created by Lewis he on April 1, 2019.
 * github:https://github.com/lewisxhe/PCF8563_Library
 */
#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include <Wire.h>
#include "pcf8563.h"
#include "rtc_date.h"
#include "rtc_alarm.h"

uint8_t PCF8563_Class::begin(TwoWire &port, uint8_t addr) {
    _i2cPort = &port;
    _address = addr;
    _i2cPort->beginTransmission(_address);

    return _i2cPort->endTransmission();
}

void PCF8563_Class::check() {
    RTC_Date now      = getDateTime();
    RTC_Date compiled = RTC_Date(__DATE__, __TIME__);

    if (
        now.year < compiled.year ||
        (now.year == compiled.year && now.month < compiled.month) ||
        (now.year == compiled.year && now.month == compiled.month && now.day < compiled.day)
    ) {
        setDateTime(compiled);
    }
}

void PCF8563_Class::setDateTime(RTC_Date date) {
    setDateTime(date.year, date.month, date.day, date.hour, date.minute, date.second);
}

uint32_t PCF8563_Class::getDayOfWeek(uint32_t day, uint32_t month, uint32_t year) {
    uint32_t val;

    if (month < 3) {
        month = 12u + month;
        --year;
    }

    val = (day + (((month + 1u) * 26u) / 10u) + year + (year / 4u) + (6u * (year / 100u)) + (year / 400u)) % 7u;
    if (0u == val) {
        val = 7;
    }

    return (val - 1);
}

void PCF8563_Class::setDateTime(
    uint16_t year,
    uint8_t  month,
    uint8_t  day,
    uint8_t  hour,
    uint8_t  minute,
    uint8_t  second
) {
    _data[0] = _dec_to_bcd(second) & (~PCF8563_VOL_LOW_MASK);
    _data[1] = _dec_to_bcd(minute);
    _data[2] = _dec_to_bcd(hour);
    _data[3] = _dec_to_bcd(day);
    _data[4] = getDayOfWeek(day, month, year);
    _data[5] = _dec_to_bcd(month);
    _data[6] = _dec_to_bcd(year % 100);

    if ((2000 % year) == 2000) {
        _data[5] &= (~PCF8563_CENTURY_MASK);
    } else {
        _data[5] |= PCF8563_CENTURY_MASK;
    }

    _writeByte(PCF8563_SEC_REG, 7, _data);
}

// [[deprecated("Use isValid() instead.")]]
__attribute__((deprecated("Use isValid() instead.")))
bool PCF8563_Class::isVaild() {
    return isValid();
}

bool PCF8563_Class::isValid() {
    _readByte(PCF8563_SEC_REG, 1, &_isValid);
    return !(_isValid & (1 << 7));
}

RTC_Date PCF8563_Class::getDateTime() {
    uint16_t year;
    uint8_t  century = 0;

    _readByte(PCF8563_SEC_REG, 7, _data);
    _voltageLow = (_data[0] & PCF8563_VOL_LOW_MASK);
    _data[0]    = _bcd_to_dec(_data[0] & (~PCF8563_VOL_LOW_MASK));
    _data[1]    = _bcd_to_dec(_data[1] & PCF8563_minuteS_MASK);
    _data[2]    = _bcd_to_dec(_data[2] & PCF8563_HOUR_MASK);
    _data[3]    = _bcd_to_dec(_data[3] & PCF8563_DAY_MASK);
    _data[4]    = _bcd_to_dec(_data[4] & PCF8563_WEEKDAY_MASK);
    century     = _data[5] & PCF8563_CENTURY_MASK;
    _data[5]    = _bcd_to_dec(_data[5] & PCF8563_MONTH_MASK);
    year        = _bcd_to_dec(_data[6]);
    year        = century ? 1900 + year : 2000 + year;

    return RTC_Date(year, _data[5], _data[3], _data[2], _data[1], _data[0]);
}

RTC_Alarm PCF8563_Class::getAlarm() {
    _readByte(PCF8563_ALRM_MIN_REG, 4, _data);
    _data[0] = _bcd_to_dec(_data[0] & (~PCF8563_minuteS_MASK));
    _data[1] = _bcd_to_dec(_data[1] & (~PCF8563_HOUR_MASK));
    _data[2] = _bcd_to_dec(_data[2] & (~PCF8563_DAY_MASK));
    _data[3] = _bcd_to_dec(_data[3] & (~PCF8563_WEEKDAY_MASK));

    return RTC_Alarm(_data[0], _data[1], _data[2], _data[3]);
}

void PCF8563_Class::enableAlarm() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    _data[0] &= ~PCF8563_ALARM_AF;
    _data[0] |= (PCF8563_TIMER_TF | PCF8563_ALARM_AIE);
    _writeByte(PCF8563_STAT2_REG, 1, _data);
}

void PCF8563_Class::disableAlarm() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    _data[0] &= ~(PCF8563_ALARM_AF | PCF8563_ALARM_AIE);
    _data[0] |= PCF8563_TIMER_TF;
    _writeByte(PCF8563_STAT2_REG, 1, _data);
}

void PCF8563_Class::resetAlarm() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    _data[0] &= ~(PCF8563_ALARM_AF);
    _data[0] |= PCF8563_TIMER_TF;
    _writeByte(PCF8563_STAT2_REG, 1, _data);
}

bool PCF8563_Class::alarmActive() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    return (bool)(_data[0] & PCF8563_ALARM_AF);
}

void PCF8563_Class::setAlarm(RTC_Alarm alarm) {
    setAlarm(alarm.minute, alarm.hour, alarm.day, alarm.weekday);
}

void PCF8563_Class::setAlarm(uint8_t hour, uint8_t minute, uint8_t day, uint8_t weekday) {
    _data[0] = PCF8563_ALARM_ENABLE;
    if (minute != PCF8563_NO_ALARM) {
        _data[0] = _dec_to_bcd(constrain(minute, 0, 59));
        _data[0] &= ~PCF8563_ALARM_ENABLE;
    }

    _data[1] = PCF8563_ALARM_ENABLE;
    if (hour != PCF8563_NO_ALARM) {
        _data[1] = _dec_to_bcd(constrain(hour, 0, 23));
        _data[1] &= ~PCF8563_ALARM_ENABLE;
    }

    if (day != PCF8563_NO_ALARM) {
        _data[2] = _dec_to_bcd(constrain(day, 1, 31));
        _data[2] &= ~PCF8563_ALARM_ENABLE;
    } else {
        _data[2] = PCF8563_ALARM_ENABLE;
    }

    if (weekday != PCF8563_NO_ALARM) {
        _data[3] = _dec_to_bcd(constrain(weekday, 0, 6));
        _data[3] &= ~PCF8563_ALARM_ENABLE;
    } else {
        _data[3] = PCF8563_ALARM_ENABLE;
    }

    _writeByte(PCF8563_ALRM_MIN_REG, 4, _data);
}

void PCF8563_Class::setAlarmByMinutes(uint8_t minute) {
    setAlarm(PCF8563_NO_ALARM, minute, PCF8563_NO_ALARM, PCF8563_NO_ALARM);
}

void PCF8563_Class::setAlarmByDays(uint8_t day) {
    setAlarm(PCF8563_NO_ALARM, PCF8563_NO_ALARM, day, PCF8563_NO_ALARM);
}

void PCF8563_Class::setAlarmByHours(uint8_t hour) {
    setAlarm(hour, PCF8563_NO_ALARM, PCF8563_NO_ALARM, PCF8563_NO_ALARM);
}

void PCF8563_Class::setAlarmByWeekDay(uint8_t weekday) {
    setAlarm(PCF8563_NO_ALARM, PCF8563_NO_ALARM, PCF8563_NO_ALARM, weekday);
}

bool PCF8563_Class::isTimerEnable() {
    _readByte(PCF8563_STAT2_REG, 1, &_data[0]);
    _readByte(PCF8563_TIMER1_REG, 1, &_data[1]);

    return _data[0] & PCF8563_TIMER_TIE && _data[1] & PCF8563_TIMER_TE;
}

bool PCF8563_Class::isTimerActive() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    return (bool)_data[0] & PCF8563_TIMER_TF;
}

void PCF8563_Class::enableTimer() {
    _readByte(PCF8563_STAT2_REG, 1, &_data[0]);
    _readByte(PCF8563_TIMER1_REG, 1, &_data[1]);
    _data[0] &= ~PCF8563_TIMER_TF;
    _data[0] |= (PCF8563_ALARM_AF | PCF8563_TIMER_TIE);
    _data[1] |= PCF8563_TIMER_TE;
    _writeByte(PCF8563_STAT2_REG, 1, &_data[0]);
    _writeByte(PCF8563_TIMER1_REG, 1, &_data[1]);
}

void PCF8563_Class::disableTimer() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    _data[0] &= ~PCF8563_TIMER_TF;
    _data[1] |= PCF8563_ALARM_AF;
    _writeByte(PCF8563_STAT2_REG, 1, _data);
}

void PCF8563_Class::setTimer(uint8_t val, uint8_t freq, bool enIntrrupt) {
    _readByte(PCF8563_STAT2_REG, 1, &_data[0]);
    _readByte(PCF8563_TIMER1_REG, 1, &_data[1]);

    if (enIntrrupt) {
        _data[0] |= 1 << 4;
    } else {
        _data[0] &= ~(1 << 4);
    }

    _data[1] |= (freq & PCF8563_TIMER_TD10);
    _data[2] = val;
    _writeByte(PCF8563_STAT2_REG, 1, &_data[0]);
    _writeByte(PCF8563_TIMER1_REG, 1, &_data[1]);
    _writeByte(PCF8563_TIMER2_REG, 1, &_data[2]);
}

void PCF8563_Class::clearTimer() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    _data[0] &= ~(PCF8563_TIMER_TF | PCF8563_TIMER_TIE);
    _data[0] |= PCF8563_ALARM_AF;
    _data[1] = 0x00;
    _writeByte(PCF8563_STAT2_REG, 1, &_data[0]);
    _writeByte(PCF8563_TIMER1_REG, 1, &_data[1]);
}

bool PCF8563_Class::enableCLK(uint8_t freq) {
    if (freq >= PCF8563_CLK_MAX) {
        return false;
    }

    _data[0] = freq | PCF8563_CLK_ENABLE;
    _writeByte(PCF8563_SQW_REG, 1, _data);

    return true;
}

void PCF8563_Class::disableCLK() {
    _data[0] = 0x00;
    _writeByte(PCF8563_SQW_REG, 1, _data);
}

const char *PCF8563_Class::formatDateTime(uint8_t sytle) {
    RTC_Date t = getDateTime();

    switch (sytle) {
        case PCF_TIMEFORMAT_HM:
            snprintf(format, sizeof(format), "%d:%d", t.hour, t.minute);
            break;

        case PCF_TIMEFORMAT_HMS:
            snprintf(format, sizeof(format), "%d:%d:%d", t.hour, t.minute, t.second);
            break;

        case PCF_TIMEFORMAT_YYYY_MM_DD:
            snprintf(format, sizeof(format), "%d-%d-%d", t.year, t.month, t.day);
            break;

        case PCF_TIMEFORMAT_MM_DD_YYYY:
            snprintf(format, sizeof(format), "%d-%d-%d", t.month, t.day, t.year);
            break;

        case PCF_TIMEFORMAT_DD_MM_YYYY:
            snprintf(format, sizeof(format), "%d-%d-%d", t.day, t.month, t.year);
            break;

        case PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S:
            snprintf(format, sizeof(format), "%d-%d-%d/%d:%d:%d", t.year, t.month, t.day, t.hour, t.minute, t.second);
            break;

        default:
            snprintf(format, sizeof(format), "%d:%d", t.hour, t.minute);
            break;
    }

    return format;
}

#ifdef ESP32
void PCF8563_Class::syncToSystem() {
    if (PCF8563_Class::isValid()) {
        struct tm t_tm;
        struct timeval val;

        RTC_Date dt  = getDateTime();
        t_tm.tm_hour = dt.hour;
        t_tm.tm_min  = dt.minute;
        t_tm.tm_sec  = dt.second;
        t_tm.tm_year = dt.year - 1900;  //Year, whose value starts from 1900
        t_tm.tm_mon  = dt.month - 1;    //Month (starting from January, 0 for January) - Value range is [0,11]
        t_tm.tm_mday = dt.day;

        val.tv_sec   = mktime(&t_tm);
        val.tv_usec  = 0;

        settimeofday(&val, NULL);

        return;
    }

    ESP_LOGE("RTC Time is not Valid", "System Epoch Not Set");
}
#endif

bool PCF8563_Class::syncToRtcUsingGmt() {
    time_t epoch;
    struct tm gmt;
    time(&epoch);

    // Is epoch is between 1970 and 2100?
    if (epoch > 0 && epoch < 4102444800) {
        gmtime_r(&epoch, &gmt);
        setDateTime(gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday, gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

        return true;
    }

    #ifdef ESP32
    ESP_LOGE("ESP32 Time is not Valid", "RTC Time Not Set");
    #endif

    return false;
}

bool PCF8563_Class::syncToRtc(bool useGmt = false) {
    if (useGmt) {
        return syncToRtcUsingGmt();
    }

    time_t now;
    struct tm info;
    time(&now);
    localtime_r(&now, &info);
    setDateTime(info.tm_year + 1900, info.tm_mon + 1, info.tm_mday, info.tm_hour, info.tm_min, info.tm_sec);

    return true;
}

uint8_t PCF8563_Class::status2() {
    _readByte(PCF8563_STAT2_REG, 1, _data);
    return _data[0];
}
