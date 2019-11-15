#include "data_transmit.h"
#include "BCD_CON.h"
#include "User_CRC8.h"
#include "Private_Sensor.h"
#include "Periph.h"
#include "memory.h"
#include "Private_RTC.h"
#include "LoRa.h"

//传感器数据缓存相关变量
unsigned char Send_Air_Sensor_Buff[128] = {0};
unsigned char Air_Data_Length = 0;
unsigned char Send_Screen_Buf[128] = {0};
unsigned char Screen_Sindex = 0;


/*
 *brief   : 发送大气和大棚的一些气象数据到服务器
 *para    : 无
 *return  : 无
 */
void Send_Air_Muti_Sensor_Data_to_Server(void)
{
  unsigned char HiByte, LoByte, flag;
  unsigned char NumOfDot = 0;
  unsigned char Data_BCD[4] = {0};
  char weathertr[20] = {0};
  float Temperature;
 
  Air_Data_Length = 0;

  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFE;
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xCC;
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x30;

  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x0C;
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x32;
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x27;
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x10;

  //大气温度
  NumOfDot = 2;
  if ((Muti_Sensor_Data.Air_Temp >= 65535) && (Muti_Sensor_Data.Air_Temp_Flag != 1))
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    if (Muti_Sensor_Data.Air_Temp_Flag == 1)
    {
      Temperature = (float)(65536 - Muti_Sensor_Data.Air_Temp) / 10.0;
    }
    else
    {
      Temperature =  (float)(Muti_Sensor_Data.Air_Temp) / 10.0;
    }

    PackBCD((char *)Data_BCD, Temperature, 4, NumOfDot);//读取回来BCD数据数组、大气温度传感器数据、数据宽度、数据小数
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];//把转换好的BCD码数据给发送缓存数组。
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }

  if(Muti_Sensor_Data.Air_Temp_Flag == 1)//判断大气温度是零上还是零下
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xF0 | NumOfDot;//最高位是1，表示负的数值
  }
  else
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;//最高位是0，表示正的数值
  }


  //大气湿度
  memset(Data_BCD, 0x00, sizeof(Data_BCD));
  NumOfDot = 2;

  if ((int)(Muti_Sensor_Data.Air_Humi) >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Air_Humi, 4, NumOfDot);//把大气湿度转换成BCD码
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;
  
  //大气光照强度
  NumOfDot = 0;

  memset(Data_BCD,0x00,sizeof(Data_BCD));
  memset(weathertr, 0x00, sizeof(weathertr));

  if (Muti_Sensor_Data.Air_Lux > 200000)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    sprintf(weathertr, "%08ld", Muti_Sensor_Data.Air_Lux);
    //该函数是将一个ASCII码字符串转换成BCD码
    ASC2BCD(Data_BCD, weathertr, strlen(weathertr));//读取BCD数据数组、ASCII码字符串、该字符串的长度

    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[2];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[3];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;
  
  //大气气压
  NumOfDot = 2;

  memset(Data_BCD,0x00,sizeof(Data_BCD));
  memset(weathertr, 0x00, sizeof(weathertr));

  sprintf(weathertr, "%08ld", Muti_Sensor_Data.Air_Atmos);
  //该函数是将一个ASCII码字符串转换成BCD码
  ASC2BCD(Data_BCD, weathertr, strlen(weathertr));//读取BCD数据数组、ASCII码字符串、该字符串的长度

  Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
  Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[2];
  Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[3];
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;

  //大气紫外线强度
  NumOfDot = 0;
  memset(Data_BCD,0x00,sizeof(Data_BCD));

  if ((int)(Muti_Sensor_Data.Air_UV) >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Air_UV, 4, NumOfDot);

    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;


  //风速
  NumOfDot = 1;
  if ((unsigned int)(Muti_Sensor_Data.Air_Wind_Speed) >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Air_Wind_Speed, 4, NumOfDot);
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;
  
  //风向
  NumOfDot = 0;
  if (Muti_Sensor_Data.Wind_DirCode >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Wind_DirCode, 4, NumOfDot);
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }

  //大气二氧化碳
  NumOfDot = 0;
  if (Muti_Sensor_Data.Air_CO2 >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Air_CO2, 4, NumOfDot);

    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;

  //大气TVOC 
  NumOfDot = 0;
  if (Muti_Sensor_Data.Air_TVOC >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Air_TVOC, 4, NumOfDot);

    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;

  //土壤温度
  NumOfDot = 2;
  memset(Data_BCD, 0x00, sizeof(Data_BCD));

  if ((Muti_Sensor_Data.Soil_Temp >= 65535) && (Muti_Sensor_Data.Soil_Temp_Flag != 1))
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    if (Muti_Sensor_Data.Soil_Temp_Flag == 1)
    {
      Temperature = (float)(65536 - Muti_Sensor_Data.Soil_Temp) / 10.0;
    }
    else
    {
      Temperature =  (float)(Muti_Sensor_Data.Soil_Temp) / 10.0;
    }

    PackBCD((char *)Data_BCD, Temperature, 4, NumOfDot);
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }

  if(Muti_Sensor_Data.Soil_Temp_Flag == 1)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xF0 | NumOfDot;
  }
  else
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;
  }
  
  //土壤湿度
  NumOfDot = 2;
  memset(Data_BCD, 0x00, sizeof(Data_BCD));

  if ((int)(Muti_Sensor_Data.Soil_Humi) >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Soil_Humi, 4, NumOfDot);
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;

  //土壤导电率
  NumOfDot = 0;
  memset(Data_BCD, 0x00, sizeof(Data_BCD));

  if (Muti_Sensor_Data.Soil_Cond >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Soil_Cond, 4, NumOfDot);
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;

  //土壤盐分
  NumOfDot = 0;
  memset(Data_BCD, 0x00, sizeof(Data_BCD));

  if (Muti_Sensor_Data.Soil_Salt >= 65535)
  {
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
    Send_Air_Sensor_Buff[Air_Data_Length++] = 0xFF;
  }
  else
  {
    PackBCD((char *)Data_BCD, Muti_Sensor_Data.Soil_Salt, 4, NumOfDot);
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[0];
    Send_Air_Sensor_Buff[Air_Data_Length++] = Data_BCD[1];
  }
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0xE0 | NumOfDot;

  //电压
  unsigned int BatVol = Get_Bat_Voltage(DEFAULT_VOL_CHANGE_TIMES);
  ToBCD(BatVol, &HiByte, &LoByte, &flag);
  Send_Air_Sensor_Buff[Air_Data_Length++] = HiByte;
  Send_Air_Sensor_Buff[Air_Data_Length++] = LoByte;

  unsigned char CRC8 = GetCrc8(Send_Air_Sensor_Buff + 3, Air_Data_Length - 3);
  Send_Air_Sensor_Buff[Air_Data_Length++] = CRC8;//CRC8校验码

  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x0D;
  Send_Air_Sensor_Buff[Air_Data_Length++] = 0x0A;

  for (unsigned char i = 0; i < Air_Data_Length; i++)
  {
    Serial.print(Send_Air_Sensor_Buff[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("LoRa Send Data to Server...");
  LoRa_Serial.write(Send_Air_Sensor_Buff, Air_Data_Length);
  delay(500);
}





