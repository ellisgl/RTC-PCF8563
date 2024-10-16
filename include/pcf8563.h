/**
 * pcf8563.h - Arduino library for NXP PCF8563 RTC chip.
 * Created by Lewis he on April 1, 2019.
 * github:https://github.com/lewisxhe/PCF8563_Library
 */
#pragma once
// Technically we shouldn't need ifndef/define/endif here, but it's just incase of any compiler oddness.
#ifndef PCF8563_H
#define PCF8563_H

#include <Arduino.h>
#include <Wire.h>
#include "rtc_alarm.h"
#include "rtc_date.h"

#define PCF8563_SLAVE_ADDRESS   (0x51) //7-bit I2C Address

//! REG MAP
#define PCF8563_STAT1_REG       (0x00)
#define PCF8563_STAT2_REG       (0x01)
#define PCF8563_SEC_REG         (0x02)
#define PCF8563_MIN_REG         (0x03)
#define PCF8563_HR_REG          (0x04)
#define PCF8563_DAY_REG         (0x05)
#define PCF8563_WEEKDAY_REG     (0x06)
#define PCF8563_MONTH_REG       (0x07)
#define PCF8563_YEAR_REG        (0x08)
#define PCF8563_ALRM_MIN_REG    (0x09)
#define PCF8563_SQW_REG         (0x0D)
#define PCF8563_TIMER1_REG      (0x0E)
#define PCF8563_TIMER2_REG      (0x0F)

#define PCF8563_VOL_LOW_MASK    (0x80)
#define PCF8563_minuteS_MASK    (0x7F)
#define PCF8563_HOUR_MASK       (0x3F)
#define PCF8563_WEEKDAY_MASK    (0x07)
#define PCF8563_CENTURY_MASK    (0x80)
#define PCF8563_DAY_MASK        (0x3F)
#define PCF8563_MONTH_MASK      (0x1F)
#define PCF8563_TIMER_CTL_MASK  (0x03)


#define PCF8563_ALARM_AF        (0x08)
#define PCF8563_TIMER_TF        (0x04)
#define PCF8563_ALARM_AIE       (0x02)
#define PCF8563_TIMER_TIE       (0x01)
#define PCF8563_TIMER_TE        (0x80)
#define PCF8563_TIMER_TD10      (0x03)

#define PCF8563_NO_ALARM        (0xFF)
#define PCF8563_ALARM_ENABLE    (0x80)
#define PCF8563_CLK_ENABLE      (0x80)

enum {
    PCF8563_CLK_32_768KHZ,
    PCF8563_CLK_1024KHZ,
    PCF8563_CLK_32HZ,
    PCF8563_CLK_1HZ,
    PCF8563_CLK_MAX
};

enum {
    PCF_TIMEFORMAT_HM,
    PCF_TIMEFORMAT_HMS,
    PCF_TIMEFORMAT_YYYY_MM_DD,
    PCF_TIMEFORMAT_MM_DD_YYYY,
    PCF_TIMEFORMAT_DD_MM_YYYY,
    PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S,
};

class PCF8563_Class
{
    public:
        uint8_t begin(TwoWire &port = Wire, uint8_t addr = PCF8563_SLAVE_ADDRESS);
        void check();
        void setDateTime(
            uint16_t year,
            uint8_t month,
            uint8_t day,
            uint8_t hour,
            uint8_t minute,
            uint8_t second
        );
        void setDateTime(RTC_Date date);
        RTC_Date getDateTime();
        RTC_Alarm getAlarm();
        void enableAlarm();
        void disableAlarm();
        bool alarmActive();
        void resetAlarm();
        void setAlarm(RTC_Alarm alarm);
        void setAlarm(uint8_t hour, uint8_t minute, uint8_t day, uint8_t weekday);
        bool isVaild();
        bool isValid();
        void setAlarmByWeekDay(uint8_t weekday);
        void setAlarmByHours(uint8_t hour);
        void setAlarmByDays(uint8_t day);
        void setAlarmByMinutes(uint8_t minute);
        bool isTimerEnable();
        bool isTimerActive();
        void enableTimer();
        void disableTimer();
        void setTimer(uint8_t val, uint8_t freq, bool enIntrrupt);
        void clearTimer();
        bool enableCLK(uint8_t freq);
        void disableCLK();
    #ifdef ESP32
        void syncToSystem();
    #endif
        bool syncToRtc(bool useGmt);
        bool syncToRtcUsingGmt();
        const char *formatDateTime(uint8_t sytle = PCF_TIMEFORMAT_HMS);
        uint32_t getDayOfWeek(uint32_t day, uint32_t month, uint32_t year);
        uint8_t status2();

    private:
        uint8_t _bcd_to_dec(uint8_t val)
        {
            return ( (val / 16 * 10) + (val % 16) );
        }

        uint8_t _dec_to_bcd(uint8_t val)
        {
            return ( (val / 10 * 16) + (val % 10) );
        }

        int _readByte(uint8_t reg, int nbytes, uint8_t *data)
        {
            _i2cPort->beginTransmission(_address);
            _i2cPort->write(reg);

            //Adapt to HYM8563, no stop bit is sent after reading the sending register address
            _i2cPort->endTransmission(false);
            _i2cPort->requestFrom(_address, nbytes, 1);  //HYM8563 send stopbit

            uint8_t index = 0;
            while (_i2cPort->available()) {
                data[index++] = _i2cPort->read();
            }

            return 0;
        }

        int _writeByte(uint8_t reg, uint8_t nbytes, uint8_t *data)
        {
            _i2cPort->beginTransmission(_address);
            _i2cPort->write(reg);
            for (uint8_t i = 0; i < nbytes; ++i) {
                _i2cPort->write(data[i]);
            }

            _i2cPort->endTransmission();

            return 0;
        }

        uint8_t _isValid = false;
        int _address;
        bool _init = false;
        TwoWire *_i2cPort;
        uint8_t _data[16];
        bool _voltageLow;
        char format[128];
};

#endif
