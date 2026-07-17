#ifndef DWT_SYSTIME_H
#define DWT_SYSTIME_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初始化高精度系统时间戳
void dwt_systime_init(void);

// 获取当前时间（纳秒）
uint64_t dwt_systime_get_ns(void);

// 获取当前时间（微秒）
uint64_t dwt_systime_get_us(void);

// 获取当前时间（毫秒）
uint64_t dwt_systime_get_ms(void);

// 替代 HAL_GetTick（返回 ms）
uint32_t dwt_systime_get_tick(void);

void dwt_systime_update(void);

void dwt_delay_ms(uint32_t ms);

void dwt_delay_us(uint32_t us); 


#ifdef __cplusplus
}
#endif

#endif