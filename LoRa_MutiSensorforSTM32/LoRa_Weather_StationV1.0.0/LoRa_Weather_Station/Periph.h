#ifndef _PERIPH_H
#define _PERIPH_H

#include "board_config.h"

//DC12V电源开、关操作
#define DC12V_PWR_ON      (digitalWrite(DC12V_PWR_PIN, HIGH))
#define DC12V_PWR_OFF     (digitalWrite(DC12V_PWR_PIN, LOW))

//RS485模块电源开、关操作
#define RS485_BUS_PWR_ON  (digitalWrite(RS485_BUS_PWR_PIN, HIGH))
#define RS485_BUS_PWR_OFF (digitalWrite(RS485_BUS_PWR_PIN, LOW))

//两个双色状态灯
#define LED1_ON           (digitalWrite(LED1, HIGH))
#define LED1_OFF          (digitalWrite(LED1, LOW))
#define LED2_ON           (digitalWrite(LED2, HIGH))
#define LED2_OFF          (digitalWrite(LED2, LOW))
#define LED3_ON           (digitalWrite(LED3, HIGH))
#define LED3_OFF          (digitalWrite(LED3, LOW))
#define LED4_ON           (digitalWrite(LED4, HIGH))
#define LED4_OFF          (digitalWrite(LED4, LOW))

//USB使能和失能操作
#define USB_PORT_EN       (digitalWrite(USB_EN_PIN,LOW))
#define USB_PORT_DIS      (digitalWrite(USB_EN_PIN,HIGH))

//GSM联网状态灯
#define GSM_STATUS_LED_ON      LED3_ON
#define GSM_STATUS_LED_OFF     LED3_OFF

//GSM连接服务器状态灯
#define Server_STATUS_LED_ON   LED4_ON
#define Server_STATUS_LED_OFF  LED4_OFF

#define DEFAULT_VOL_CHANGE_TIMES    11


void EP_Write_Enable(void);
void EP_Write_Disable(void);

void Some_GPIO_Init(void);
unsigned int Get_Bat_Voltage(unsigned char change_times);

#endif