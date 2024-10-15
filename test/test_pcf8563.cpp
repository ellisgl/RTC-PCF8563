#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include <Wire.h>
#include "pcf8563.h"
#include "unity.h"


PCF8563_Class rtc;
// TwoWire Wire = TwoWire(0);

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

void test_stuff(void);

void setup()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN(); // IMPORTANT LINE!
    RUN_TEST(test_stuff);
    UNITY_END(); // stop unit testing
}

void loop(void)
{
    // run tests
}

void test_stuff(void)
{
    // test stuff

    pinMode(23, PULLUP);
    Wire.begin(21, 22);
    rtc.begin();
    rtc.setDateTime(2019, 4, 1, 12, 33, 59);
    const char* dt = rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S);
    TEST_ASSERT_EQUAL_STRING("2019-4-1/12:33:59", dt);

    rtc.setDateTime(2020, 5, 2, 11, 32, 58);
    const char* dt2 = rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S);
    TEST_ASSERT_EQUAL_STRING("2020-5-2/11:32:58", dt2);

    delay(3000);
    const char* dt3 = rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S);
    TEST_ASSERT_EQUAL_STRING("2020-5-2/11:33:1", dt2);

}