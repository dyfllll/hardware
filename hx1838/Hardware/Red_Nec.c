#include "Red_Nec.h"
#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include "common.h"

/* 环形缓冲区: 用来保存解析出来的按键,可以防止丢失 */
#define BUF_LEN 128
static unsigned char g_KeysBuf[BUF_LEN];
static int g_KeysBuf_R, g_KeysBuf_W;

#define NEXT_POS(x) ((x + 1) % BUF_LEN)

static uint8_t isKeysBufEmpty(void)
{
    return (g_KeysBuf_R == g_KeysBuf_W);
}

static uint8_t isKeysBufFull(void)
{
    return (g_KeysBuf_R == NEXT_POS(g_KeysBuf_W));
}

static uint8_t KeysBufSize(void)
{
    uint8_t size;
    if (g_KeysBuf_W >= g_KeysBuf_R)
    {
        size = g_KeysBuf_W - g_KeysBuf_R;
    }
    else
    {
        size = BUF_LEN - (g_KeysBuf_R - g_KeysBuf_W);
    }
    return size;
}

static void PutKeyToBuf(unsigned char key)
{
    if (!isKeysBufFull())
    {
        g_KeysBuf[g_KeysBuf_W] = key;
        g_KeysBuf_W = NEXT_POS(g_KeysBuf_W);
    }
}

static unsigned char GetKeyFromBuf(void)
{
    unsigned char key = 0xff;
    if (!isKeysBufEmpty())
    {
        key = g_KeysBuf[g_KeysBuf_R];
        g_KeysBuf_R = NEXT_POS(g_KeysBuf_R);
    }
    return key;
}

typedef enum
{
    RN_Init,
    RN_Start_High_9ms,
    RN_Start_Low_4_5ms,
    RN_Read_State0,
    RN_Read_State1,
    RN_Repeat,
    RN_End,

} Red_Nec_Rec_State;

static Red_Nec_Rec_State g_Last_State;
static uint8_t g_Last_Value;
static uint64_t g_Last_Time;
static uint32_t g_Receive_Buffer;
static uint8_t g_Receive_Count;
static uint8_t g_Last_Addr, g_Last_Data;
#define g_Receive_MaxCount 32


uint8_t Appro(uint64_t current, uint64_t last, int32_t target)
{
    return abs((int32_t)(current - last) - target) < 300;
}

void Red_Nec_IRQ_Callback(void)
{
    uint64_t currentTime = DWT_SysTime_Get_us();

    switch (g_Last_State)
    {
    case RN_Init:
        g_Last_State = RN_Init;
        g_Receive_Buffer = 0;
        g_Receive_Count = 0;
        if (g_Last_Value == 0 && Appro(currentTime, g_Last_Time, 9000))
        {
            g_Last_State = RN_Start_High_9ms;
        }
        break;
    case RN_Start_High_9ms:
        g_Last_State = RN_Init;
        if (g_Last_Value == 1 && Appro(currentTime, g_Last_Time, 4500))
        {
            g_Last_State = RN_Start_Low_4_5ms;
        }
        else if (g_Last_Value == 1 && Appro(currentTime, g_Last_Time, 2250))
        {
            g_Last_State = RN_Repeat;
        }
        break;
    case RN_Start_Low_4_5ms:
    case RN_Read_State1:
        g_Last_State = RN_Init;
        if (g_Last_Value == 0 && Appro(currentTime, g_Last_Time, 560))
        {
            g_Last_State = RN_Read_State0;
        }
        break;
    case RN_Read_State0:
        g_Last_State = RN_Init;

        if (g_Last_Value == 1)
        {
            if (Appro(currentTime, g_Last_Time, 560)) // 接收到0
            {
                g_Receive_Count++;
                if (g_Receive_Count >= g_Receive_MaxCount)
                    g_Last_State = RN_End;
                else
                    g_Last_State = RN_Read_State1;
            }
            else if (Appro(currentTime, g_Last_Time, 1680)) // 接收到1
            {
                //|addr|~addr|data|~data|
                g_Receive_Buffer |= 0x80000000 >> g_Receive_Count;
                g_Receive_Count++;
                if (g_Receive_Count >= g_Receive_MaxCount)
                    g_Last_State = RN_End;
                else
                    g_Last_State = RN_Read_State1;
            }
        } 
        break;

    case RN_End:
        g_Last_State = RN_Init;
        if (g_Last_Value == 0)
        {
            uint8_t *data_array = (uint8_t *)&g_Receive_Buffer;
            uint8_t addr, data;
            if (data_array[3] == (uint8_t)(~data_array[2]) && data_array[1] == (uint8_t)(~data_array[0]))
            {
                addr = data_array[3];
                data = data_array[1];
                PutKeyToBuf(addr);
                PutKeyToBuf(data);
                g_Last_Addr = addr;
                g_Last_Data = data;
            }
        }
        break;
    case RN_Repeat:
        g_Last_State = RN_Init;
        if (g_Last_Value == 0)
        {
            PutKeyToBuf(g_Last_Addr);
            PutKeyToBuf(g_Last_Data);
        }
        break;

    default:
        break;
    }

    g_Last_Time = currentTime;
    g_Last_Value = !g_Last_Value;
}

void Red_Nec_Init(void)
{
    // 配置为双边沿触发, 并使能了中断
    // 默认上拉
    g_Last_Value = 1;
    g_Last_Time = 0;
    g_Last_State = RN_Init;
}

int Red_Nec_Read(uint8_t *pDev, uint8_t *pData)
{
    if (isKeysBufEmpty())
        return -1;

    *pDev = GetKeyFromBuf();
    *pData = GetKeyFromBuf();
    return 0;
}

const char *Red_Nec_CodeToString(uint8_t code)
{
    const char *result;
    switch (code)
    {
    case 162:
        result = "1";
        break;
    case 98:
        result = "2";
        break;
    case 226:
        result = "3";
        break;
    case 34:
        result = "4";
        break;
    case 2:
        result = "5";
        break;
    case 194:
        result = "6";
        break;
    case 224:
        result = "7";
        break;
    case 168:
        result = "8";
        break;
    case 144:
        result = "9";
        break;
    case 152:
        result = "0";
        break;
    case 104:
        result = "*";
        break;
    case 176:
        result = "#";
        break;
    case 24:
        result = "^";
        break;
    case 16:
        result = "<";
        break;
    case 74:
        result = "v";
        break;
    case 90:
        result = ">";
        break;
    case 56:
        result = "OK";
        break;
    default:
        result = "Error";
        break;
    }
    return result;
}
