#include "common.h"
#include "stm32f1xx_hal.h"

// 0 用SysTick
// 1 用TIM4
void udelay(int us)
{
#if 0    
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000);  /* 假设reload对应1ms */
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
#else
    extern TIM_HandleTypeDef htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;

    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ticks = us * reload / (1000); /* 假设reload对应1ms */
    told = __HAL_TIM_GET_COUNTER(hHalTim);
    while (1)
    {
        tnow = __HAL_TIM_GET_COUNTER(hHalTim);
        if (tnow != told)
        {
            if (tnow > told)
            {
                tcnt += tnow - told;
            }
            else
            {
                tcnt += reload - told + tnow;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
#endif
}

void mdelay(int ms)
{
    for (int i = 0; i < ms; i++)
        udelay(1000);
}

uint32_t system_get_ms(void)
{
    return HAL_GetTick();
}

uint64_t system_get_us(void)
{
    extern TIM_HandleTypeDef htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;

    uint64_t ns = HAL_GetTick();
    uint64_t cnt;
    uint64_t reload;

    cnt = __HAL_TIM_GET_COUNTER(hHalTim);
    reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ns *= 1000;
    ns += cnt * 1000 / reload;
    return ns;
}

// uint64_t system_get_us(void)
// {
//     extern TIM_HandleTypeDef htim4;
//     TIM_HandleTypeDef *hHalTim = &htim4;

//     uint32_t tick1, tick2;
//     uint32_t cnt;
//     uint32_t reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

//     do
//     {
//         tick1 = HAL_GetTick();
//         cnt = __HAL_TIM_GET_COUNTER(hHalTim);
//         tick2 = HAL_GetTick();
//     } while (tick1 != tick2);

//     uint64_t us = tick1 * 1000ULL;
//     us += ((uint64_t)cnt * 1000ULL) / reload;
//     return us;
// }

// uint64_t system_get_us(void)
// {
//     extern TIM_HandleTypeDef htim4;

//     uint32_t primask = __get_PRIMASK(); // 读中断状态
// 	__disable_irq();
// 	uint64_t tick = HAL_GetTick();
//     uint32_t cnt = __HAL_TIM_GET_COUNTER(&htim4);
//     uint32_t reload = __HAL_TIM_GET_AUTORELOAD(&htim4);

// 	__set_PRIMASK(primask); // 恢复原状态

//     uint64_t us = tick * 1000ULL + ((uint64_t)cnt * 1000ULL) / reload;
//     return us;
// }

uint64_t system_get_ns(void)
{
    // extern uint32_t HAL_GetTick(void);
    extern TIM_HandleTypeDef htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;

    uint64_t ns = HAL_GetTick();
    uint64_t cnt;
    uint64_t reload;

    cnt = __HAL_TIM_GET_COUNTER(hHalTim);
    reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ns *= 1000000;
    ns += cnt * 1000000 / reload;
    return ns;
}
