#ifndef DWT_SYSTIME_H
#define DWT_SYSTIME_H

#include <stdint.h>
#include "stm32f1xx_hal.h"   

#ifdef __cplusplus
extern "C" {
#endif

// 初始化高精度系统时间戳
void DWT_SysTime_Init(void);

// 获取当前时间（纳秒）
uint64_t DWT_SysTime_Get_ns(void);

// 获取当前时间（微秒）
uint64_t DWT_SysTime_Get_us(void);

// 获取当前时间（毫秒）
uint64_t DWT_SysTime_Get_ms(void);

// 替代 HAL_GetTick（返回 ms）
uint32_t DWT_SysTime_GetTick(void);

#ifdef __cplusplus
}
#endif

#endif