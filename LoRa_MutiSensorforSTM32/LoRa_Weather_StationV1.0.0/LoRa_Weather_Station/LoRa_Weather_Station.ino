
#include <Arduino.h>
#include <libmaple/nvic.h>
#include <libmaple/pwr.h>
#include <libmaple/bkp.h>
#include "memory.h"
#include "Private_Sensor.h"
#include "data_transmit.h"
#include "private_delay.h"
#include "Periph.h"
#include "Private_RTC.h"
#include "LoRa.h"
#include "User_CRC8.h"

unsigned int Run_Time_Out_Sec = 0;          //运行超时计数
char Toggle = 0;                             //状态灯闪灭位翻转
bool CollectDataFlag = false;

void setup()
{
  Some_GPIO_Init();//IO口初始化定义

  Serial.begin(9600); //USB serial port.
  ModBus_Serial.begin(9600);
  LoRa_MHL9LF.BaudRate(9600);
  
  LoRa_MHL9LF.LoRa_GPIO_Config();
  LoRa_MHL9LF.Mode(PASS_THROUGH_MODE);

  Key_Clear_LoRa_Param();

  // delay(5000);
  // Clear_LoRa_Config_Flag();

  if (Verify_LoRa_Config_Flag() == false)
  {
    LoRa_MHL9LF.Parameter_Init();
    //LoRa_MHL9LF.IsReset(true);
    LoRa_MHL9LF.Mode(PASS_THROUGH_MODE);
    Save_LoRa_Config_Flag();
  }
  
  bkp_init();

  //如果电池电压小于6.5V，设备休眠
  if(Get_Bat_Voltage(DEFAULT_VOL_CHANGE_TIMES) < MIN_BAT_VOL)
    SYS_Sleep();

  //初始化定时器2
  Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
  Timer2.setPeriod(1000000); // in microseconds，1S
  Timer2.setCompare1(1);   // overflow might be small
  Timer2.attachCompare1Interrupt(Time2_Handler);

  //RS485模块开始采集传感器数据
  CollectDataFlag = true;
  Data_Acquisition();
  CollectDataFlag = false;
} 

void loop()
{
  if(Get_Bat_Voltage(DEFAULT_VOL_CHANGE_TIMES) < MIN_BAT_VOL)
    SYS_Sleep();

  LED3_ON;
  Send_Air_Muti_Sensor_Data_to_Server();

  //初始化RTC闹钟
  Init_RTC(60); //休眠60S
  SYS_Sleep();
}

/*
 *brief   : 关闭相关电源，进入待机模式
 *para    : 无
 *return  :
 */
void SYS_Sleep(void)
{
  Timer2.detachCompare1Interrupt();
  LED1_OFF;
  LED2_OFF;
  LED3_OFF;
  LED4_OFF;
  DC12V_PWR_OFF;
  USB_PORT_DIS;

  PWR_WakeUpPinCmd(ENABLE);//使能唤醒引脚，默认PA0
  PWR_ClearFlag(PWR_FLAG_WU);
  PWR_EnterSTANDBYMode();//进入待机
}

/*
 *brief   : 按键2清除储存的系统相关参数
 *para    : 无
 *return  : 无
 */
void Key_Clear_Device_Parameter(void)
{
  if (digitalRead(KEY2_INPUT) == LOW){
    GSM_STATUS_LED_ON;
    Delay_ms(1000);
    if (digitalRead(KEY2_INPUT) == LOW){
      Clear_HostID = true;
      Serial.println("Clear System parameter OK...");
      GSM_STATUS_LED_OFF; 
    } 
  }
}

void Key_Clear_LoRa_Param(void)
{ 
  if (digitalRead(KEY1_INPUT) == LOW){
    GSM_STATUS_LED_ON;
    Delay_ms(1000);
    if (digitalRead(KEY1_INPUT) == LOW){
      Clear_LoRa_Config_Flag();
      Serial.println("Clear LoRa Para OK...");
      GSM_STATUS_LED_OFF; 
    }
  }
}  


/*
 *brief   : 定时器2中断函数
 *para    : 无
 *return  : 无
*/
void Time2_Handler(void)
{
    Toggle ^= 1;
    if (CollectDataFlag)
      digitalWrite(LED1, Toggle); //状态灯闪烁
    else
      LED1_OFF;

    Run_Time_Out_Sec++;
    //如果运行超时，复位
    if(Run_Time_Out_Sec >= 300){
        Run_Time_Out_Sec = 0;
        noInterrupts();
        nvic_sys_reset();
    }
}
