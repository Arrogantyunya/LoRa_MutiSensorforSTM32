#include "LoRa.h"
#include "BCD_CON.h"
#include "public.h"

/*Create LoRa object*/
LoRa LoRa_MHL9LF;

/*
 @brief     : 配置LoRa模块相关引脚
 @param     : 无
 @return    : 无
 */
void LoRa::LoRa_GPIO_Config(void)
{
    pinMode(LORA_PWR_PIN, OUTPUT);
    LORA_PWR_ON;
    pinMode(AT_CMD_PIN, OUTPUT);
    pinMode(WAKEUP_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(WAKEUP_PIN, LOW);
    digitalWrite(RESET_PIN, HIGH);
}

/*
 @brief     : 配置LoRa串口波特率
 @param     : baudrate(such as 4800, 9600, 115200)
 @return    : 无
 */
void LoRa::BaudRate(unsigned int baudrate)
{
    LoRa_Serial.begin(baudrate);
}

/*
 @brief     : 配置LoRa模式。
 @param     : AT_status : 高电平是AT模式，低电平是透传模式
 @return    : 无
 */
void LoRa::Mode(LoRa_Mode AT_status)
{
    AT_status == AT ? digitalWrite(AT_CMD_PIN, HIGH) : digitalWrite(AT_CMD_PIN, LOW);
    delay(100);
}

/*
 @brief     : 决定LoRa是否重启
 @param     : Is_reset  : 软件重启LoRa模块。拉低不少于150ms，然后给一个高电平
 @return    : 无
 */
void LoRa::IsReset(bool Is_reset)
{
    if (Is_reset == true)
    {
        digitalWrite(RESET_PIN, LOW);
        delay(150); //150ms
        digitalWrite(RESET_PIN, HIGH);
    }
}

/*
 @brief     : 将LoRa模块完全断电
 @param     : 无
 @return    : 无
 */
void LoRa::LoRa_Shutdown(void)
{   
    LORA_PWR_OFF;
    /*注意！，如果下面这个end()已经执行过一次，并且中途没有重启打开begin，第二次end()，会死机！*/
    LoRa_Serial.end();
    pinMode(PA2, OUTPUT); //TX
    pinMode(PA3, OUTPUT); //RX
    digitalWrite(PA2, LOW);
    digitalWrite(PA3, LOW);
    digitalWrite(RESET_PIN, LOW);    
}

/*
 @brief     : 将完全断电的LoRa模块重启
 @param     : 无
 @return    : 无
 */
void LoRa::LoRa_Restart(void)
{
    LORA_PWR_ON;
    BaudRate(9600);
    //digitalWrite(RESET_PIN, HIGH);
    IsReset(true);
    Mode(PASS_THROUGH_MODE);
}

/*
 @brief     : 检测是否收到错误回执
 @param     : 1.要检验的数据缓存
              2.接收信息缓存
 @return    : 错误或正确
 */
unsigned char LoRa::Detect_Error_Receipt(unsigned char *verify_data, unsigned char *data_buffer)
{
    unsigned char Error;
    if (verify_data[0] == 'E' && verify_data[1] == 'R')
    {
        Error = (verify_data[2] - '0') * 10 + (verify_data[3] - '0');   //解析出错误代号，如：01, 02等
        Serial.println(Error);  //打印出该错误代号
        return Error;
    }
    else
        return No_Err;
}

/*
 @brief     : 用于设置LoRa参数后，如果设置参数成功，接收到回执“OK”
 @param     : 1.要验证的数据缓存
              2.接收信息缓存   
 @return    : true or false                    
 */
bool LoRa::Detect_OK_Receipt(unsigned char *verify_data, unsigned char *data_buffer)
{
    /*校验接收回执，是否成功设置参数*/
    if (verify_data[0] == 'O' && verify_data[1] == 'K')
    {
        data_buffer[0] = verify_data[0];
        data_buffer[1] = verify_data[1];
        return true;
    }
    else
        return false;
}

/*
 @brief     : LoRa模块核心函数，用来发送AT指令给LoRa模块，同时也包含了判断是否接到查询数据、或设置OK，
              或设置失败等信息。该函数用来查询参数。
 @param     : 1.AT命令
              2.接收返回的信息
              3.接收信息长度
 @return    : received data type (bytes, error, OK)
 */
Receive_Type LoRa::AT_Query_Cmd(char *cmd, unsigned char *data_buffer, unsigned char *data_len = 0)
{
    unsigned char ReceiveLen = 0, CopyLen = 0;
    unsigned char AddrTemp[100] = {0};
    bool IsValid = false;

    /*发送指令给LoRa，同时接收返回的数据*/
    LoRa_Serial.print(cmd);
    delay(100);
    while (LoRa_Serial.available() > 0)
    {
        if (ReceiveLen >= 100) return Invalid;
        AddrTemp[ReceiveLen++] = LoRa_Serial.read();
    }
    /*判断接收的数据长度是否有效*/
    ReceiveLen > 0 ? IsValid = true : IsValid = false;

    if (ReceiveLen == 0)
        Serial.println("No receive data !!!");

    if (IsValid)
    {
        /*验证帧头帧尾格式是否合法*/
        if ((AddrTemp[0] == '\r' && AddrTemp[1] == '\n') && (AddrTemp[ReceiveLen - 1] == '\n' && AddrTemp[ReceiveLen - 2] == '\r'))
        {
            /*接收除了帧头帧尾的有效数据*/
           for (unsigned char i = 2; i < ReceiveLen - 2; i++)
               AddrTemp[CopyLen++] =  AddrTemp[i];

            /*判断接收的回执是否是ERROR*/
           if (Detect_Error_Receipt(&AddrTemp[0], data_buffer) != No_Err) return ERROR;
 
           /*分析接收的回执*/
           if(Parse_Command(AddrTemp, CopyLen, data_buffer, &data_len) == true)
               return Bytes;
            else
                return Invalid;
        }
        else
            return Invalid;
    }
    else
        return Invalid;
}

/*
 @brief     : LoRa模块核心函数，用来发送AT指令给LoRa模块，同时也包含了判断是否接到查询数据、或设置OK，
              或设置失败等信息。
              该函数用来设置参数
 @param     : 1.AT指令
              2.要设置的参数
              3.接收的数据缓存
 @return    : received data type (bytes, error, OK)
 */
Receive_Type LoRa::AT_Config_Cmd(char *cmd, char * para, unsigned char *data_buffer)
{
    /*参考 AT_Query_Cmd 函数*/
    unsigned char ReceiveLen = 0, CopyLen = 0;
    unsigned char AddrTemp[100] = {0};
    char AT_Cmd[100] = {0};
    unsigned char i = 0, j = 0;
    bool IsValid = false;

    for (; cmd[i] != '\0'; i++)
        AT_Cmd[i] = cmd[i];

    for (; para[j] != '\0'; j++)
        AT_Cmd[i++] = para[j];

    /*加入AT指令帧尾格式*/
    AT_Cmd[i++] = '\r';
    AT_Cmd[i++] = '\n';

    Serial.write(AT_Cmd);

    /*发送指令给LoRa，同时接收返回的数据*/
    LoRa_Serial.write(AT_Cmd, i);
    delay(100);
    while (LoRa_Serial.available() > 0)
    {
        if (ReceiveLen >= 100) return Invalid;
        AddrTemp[ReceiveLen++] = LoRa_Serial.read();
    }
    ReceiveLen > 0 ? IsValid = true : IsValid = false;

    if (IsValid)
    {
        if ((AddrTemp[0] == '\r' && AddrTemp[1] == '\n') && (AddrTemp[ReceiveLen - 1] == '\n' && AddrTemp[ReceiveLen - 2] == '\r'))
        {
           for (unsigned char i = 2; i < ReceiveLen - 2; i++)
           {
               AddrTemp[CopyLen++] =  AddrTemp[i];
           }
           if (Detect_Error_Receipt(&AddrTemp[0], data_buffer) != No_Err) return ERROR;
           if (Detect_OK_Receipt(&AddrTemp[0], data_buffer) == true) return OK;
        }
        else
            return Invalid;
    }
    else
        return Invalid;
}

/*
 @brief     : 在AT指令查询参数后，用于判断返回的参数是何种参数，然后传递给对应的函数解析获取参数
 @param     : 1.接收的回执信息
              2.回执信息的长度
              3.返回分析的数据缓存
              4.返回分析的数据缓存长度
 @return    : true or false
 */
bool LoRa::Parse_Command(unsigned char *addr_temp, unsigned char len, unsigned char *data_buffer, unsigned char **data_len)
{
    unsigned char WhichCmd;
    unsigned char i = 0, j = 0;

    /*判断是否命令是查询信号强度命令*/
    if (addr_temp[1] == 'C' && addr_temp[2] == 'S' && addr_temp[3] == 'Q')
        WhichCmd = CSQ;
    else //或者是其他命令
        WhichCmd = CMOMON;

    /*回执的有效数据在 : 后面，所以这里截取 : 后面的数据*/
    while (addr_temp[i] != ':')
        i++;
    i++;
    for (; i <= len; i++)
        addr_temp[j++] = addr_temp[i];
    
    switch (WhichCmd)
    {
        case CMOMON : **data_len = (j -1) / 2; Get_Bytes(addr_temp, j - 1, data_buffer); break;
        case CSQ    : **data_len = 2; Get_CSQ(addr_temp, j - 1, data_buffer); break;
    }
    return true;
}

/*
 @brief     : 得到一串的16进制参数，如单播地址，组播地址等。
 @param     : 1.接收的数据回执
              2.接收数据回执的长度
              3.返回分析的数据缓存
 @return    : 无
 */
void LoRa::Get_Bytes(unsigned char *addr_temp, unsigned char len, unsigned char *data_buffer)
{
    bool SingleHexFlag = false;
    String_to_Hex(&addr_temp[0], len);

    for (unsigned char i = 0, j = 0; i <= len; i++)
    {
        if (i % 2 == 0)
        {
            data_buffer[j] = addr_temp[i] * 10;
            data_buffer[j] == 0 ? SingleHexFlag = true : SingleHexFlag = false;
        }
        else
        {
            data_buffer[j++] += addr_temp[i];
            if (SingleHexFlag == false)
               Type_Conv.Hex_To_Dec(&data_buffer[j - 1]);
        }
    }
}

/*
 @brief     : 得到信噪比和接收信号强度参数。
 @param     : 1.接收的数据回执
              2.接收数据回执的长度
              3.返回分析的数据缓存
 @return    : 无
 */
void LoRa::Get_CSQ(unsigned char *addr_temp, unsigned char len, unsigned char *data_buffer)
{
    unsigned char i = 0, j = 0;

    /*信噪比和接收强度两个值中间有逗号隔开，这里去除逗号，得到数值*/
    for (; addr_temp[i] != ','; i++)
        data_buffer[i] = addr_temp[i];

    data_buffer[i++] = 0x55;

    for (j = i; j < len; j++)
        data_buffer[j] = addr_temp[j];
}

/*
 @brief     : LoRa返回的回执信息是ASCLL码，将ASCLL码转换成HEX。
 @param     : 1.ASCLL码
              2.ASCLL码长度
 @return    : true or false
 */
bool LoRa::String_to_Hex(unsigned char *str, unsigned char len)
{
    for (unsigned char i = 0; i < len; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
            str[i] -= '0';
        else if (str[i] >= 'A' && str[i] <= 'F')
            str[i] -= '7';
        else if (str[i] >= 'a' && str[i] <= 'f')
            str[i] -= 'W';
        else
            return false;
    }
    return true;
}

/*
 @brief     : 发送AT指令接口。可以通过该函数设置LoRa模块，或是查询设置信息。
 @param     : 1.接收返回的数据
              2.是查询指令还是设置指令
              3.命令名字
              4.命令参数（如果是查询指令，忽略命令参数）
 @return    : true or false
 */
bool LoRa::LoRa_AT(unsigned char *data_buffer, bool is_query, char *cmd, char *para)
{
    unsigned char ReceiveLength = 0;
    unsigned char ReturnType;

    Mode(AT);

    /*根据指令是查询指令还是设置参数指令，分别进入不同的指令函数*/
    if (is_query == true)
        ReturnType = AT_Query_Cmd(cmd, data_buffer, &ReceiveLength);
    else
        ReturnType = AT_Config_Cmd(cmd, para, data_buffer);

    /*根据指令执行情况，返回对应状态。*/
    switch (ReturnType)
    {
        case OK     : Serial.println("Set para OK");       Mode(PASS_THROUGH_MODE); return true; break;
        case ERROR  : Serial.println("Receipt ERROR...");  Mode(PASS_THROUGH_MODE); return false; break;
        case Bytes  : 
                    for (unsigned char i = 0; i < ReceiveLength; i++)
                    {
                    Serial.print(data_buffer[i], HEX);
                    Serial.print(" ");
                    }
                    Serial.println();
                    Mode(PASS_THROUGH_MODE);
                    return true; 
                    break;

        case Invalid : Mode(PASS_THROUGH_MODE); return false; break;

        default     : Mode(PASS_THROUGH_MODE);  return true; break;
    }
}

/*
 @brief     : 由于对该LoRa模块产商出产配置的初始LoRa地址极其不信任，故该函数的功能就是
              将LoRa地址读取出来，再写入进去。确保“表里如一”。
 @param     : 无
 @return    : 无
 */
bool LoRa::Rewrite_ID(void)
{
    unsigned char RcvBuffer[4];
    char WriteAddr[9];
    unsigned char i, j = 0;

    /*读取LoRa通信地址*/
    LoRa_AT(RcvBuffer, true, AT_ADDR_, 0);

    /*从DEC转换成HEX*/
    for (unsigned char i = 0; i < 8; i++)
        i % 2 == 0 ? WriteAddr[i] = RcvBuffer[j] / 16 : WriteAddr[i] = RcvBuffer[j++] % 16;

    for (unsigned char i = 0; i < 8; i++)
    {
        if (WriteAddr[i] >= 0 && WriteAddr[i] <= 9)
            WriteAddr[i] += '0';
        else if (WriteAddr[i] >= 10 && WriteAddr[i] <= 15)
            WriteAddr[i] += '7';
    }
    WriteAddr[8] = '\0';

    /*写入读出来的地址*/
    if(!LoRa_AT(RcvBuffer, false, AT_ADDR, WriteAddr))
        return false;
    
    return true;
}

/*
 @brief     : 由于对该LoRa模块产商出产配置的初始LoRa广播地址极其不信任，故该函数的功能就是
              将LoRa广播地址读取出来，再写入进去。确保“表里如一”。
 @param     : 无
 @return    : 无
 */
bool LoRa::Rewrite_GroupID(void)
{
    unsigned char RcvBuffer[4] = {0};
    char WriteAddr[9];
    LoRa_AT(RcvBuffer, true, AT_MADDR_, 0);

    /*如果广播地址是我公司规定的，则无需重复写入*/
    if (RcvBuffer[0] == 0x71 && RcvBuffer[1] == 0x00 && RcvBuffer[2] == 0x00 && RcvBuffer[3] == 0x00)
        return true;

    WriteAddr[0] = '7';
    WriteAddr[1] = '1';
    for (unsigned char i = 2; i < 8; i++)
        WriteAddr[i] = '0';

    WriteAddr[8] = '\0';

    if(!LoRa_AT(RcvBuffer, false, AT_MADDR, WriteAddr))
        return false;
    
    return true;
}

/*
 @brief     : 初始化LoRa配置。如果没有配置，无法与网关通信。
 @param     : 无
 @return    : 无
 */
void LoRa::Parameter_Init(void)
{
    Serial.println("Configurate LoRa parameters...");
    unsigned char RcvBuffer[10];
    unsigned char StatusBuffer[15] = {0};
    bool SetStatusFlag;
    unsigned char i = 0, j;
    unsigned char TryNum = 0;

    do{
        i = 0;
        SetStatusFlag = true;

        StatusBuffer[i++] = Rewrite_GroupID();
        StatusBuffer[i++] = Rewrite_ID();
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_TFREQ, "1C578DE0"); //发射频率（HEX）475.5MHz
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_RFREQ, "1C03AE80"); //接收频率（HEX）470MHz
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_RIQ, "00"); //接收载波不反转（默认）
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_NET, "00"); //模块与模块之间通信
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_TSF, "09"); //发送扩频值：09
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_RSF, "09"); //接收扩频值：09
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_SIP, "00"); //不带包序不带地址
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_BW, "07"); //带宽值：125K
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_POW, "14"); //发射功率值（HEX）为20dBm
        StatusBuffer[i++] = LoRa_AT(RcvBuffer, false, AT_TIQ, "00"); //发送载波不反转（默认）  

        for (j = 0; j < i; j++)
        {
            if (StatusBuffer[j] == 0)
            {
                Serial.println("LoRa parameters set ERROR! <Parameter_Init>");
                SetStatusFlag = false;
                TryNum++;
                LoRa_Restart();
                LoRa_Shutdown();
                delay(2000);
                break;
            }
        }
        LoRa_Restart();

    }while (!SetStatusFlag && TryNum < 10);

    /*
      *如果连续尝试100次配置LoRa参数都失败，说明LoRa模块已损坏。
      *直接进入死循环，同时LoRa参数错误灯状态闪烁，等待维修或更换设备！
     */
    if (!SetStatusFlag)
    {
        while (1)
        {
            Serial.println("Set LoRa paramter ERROR, please check the LoRa module <Parameter_Init>");
            delay(3000);      
        }
    }
}