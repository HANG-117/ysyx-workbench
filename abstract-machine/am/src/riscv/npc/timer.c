#include <am.h>
#define RTC_ADDR_LOW 0xa0000048
#define RTC_ADDR_HIGH 0xa000004c

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = 0;
  volatile uint32_t *rtc = (volatile uint32_t *)(RTC_ADDR_LOW);
  uint32_t hi = rtc[1];
  uint32_t lo = rtc[0]; // 读取低32位时触发 rtc_io_handler 更新
  uptime->us = ((uint64_t)hi << 32) | lo;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
