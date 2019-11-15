#include "Private_RTC.h"
#include <Arduino.h>
#include "data_transmit.h"


UTCTimeStruct RtcTime;
RTClock InRtc (RTCSEL_LSE);  // initialise RTC

void Init_RTC(unsigned int init_time)
{
    time_t Alarm_Time = 0;
    Alarm_Time = InRtc.getTime();
    Alarm_Time += init_time;
    InRtc.createAlarm(RTC_Interrupt, Alarm_Time);
}

/*
 *brief   : RTC闹钟中断函数，进入后接触RTC闹钟中断，系统重启
 *para    : 无
 *return  : 无
*/
void RTC_Interrupt(void)
{
  rtc_detach_interrupt(RTC_ALARM_SPECIFIC_INTERRUPT);
  nvic_sys_reset();
}