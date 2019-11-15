#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define TYPE06    1 //棚内+大气
#define TYPE07    0 //中科院带三路ADC
#define TYPE08    0 //第三代气象站，新增光学雨量计

#define PRINT_DEBUG   0

//EEPROM capacity
#define TABLE_SIZE 131072 

//Serial port config.
#define ModBus_Serial               Serial2
#define LoRa_Serial                 Serial1

//Button config
#define KEY1_INPUT                  PA4
#define KEY2_INPUT                  PA5

//LED config
#define LED1                        PC2
#define LED2                        PC3
#define LED3                        PC0
#define LED4                        PC1

//External ADC config
#define HOST_VOL_ADC_INPUT_PIN      PB1

//外设电源、信号控制IO,输出控制
#define DC12V_PWR_PIN               PB12
#define RS485_BUS_PWR_PIN           PB13

//EEPROM W/R enable pin.
#define WP_PIN                      PB5

//USB使能脚
#define USB_EN_PIN                  PB15

#define ANALOGY_PIN_1               PC4
#define ANALOGY_PIN_2               PA7

//ADC定义,12位ADC，参考电压为3.3V
#define ADC_RATE                            0.8056 //3300/4096
//电池输入电压分压比
#define VBAT_DIVIDER_RATIO                  6  

#define MIN_BAT_VOL                         6800

#endif