#include "dwt_systime.h"

static uint32_t last_cycle = 0;
static uint64_t high_cycle = 0;

void dwt_systime_init(void)
{
    // 启用 DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    last_cycle = 0;
    high_cycle = 0;
}

// 获取 64 位 cycle，处理溢出
uint64_t dwt_get_cycle64(void)
{
    uint32_t now = DWT->CYCCNT;
    if (now < last_cycle)
    {
        high_cycle += (1ULL << 32); // 溢出补偿
    }
    last_cycle = now;
    return high_cycle | now;
}

// 必须在溢出前调用，否则会丢失时间
void dwt_systime_update(void)
{
    dwt_get_cycle64();
}

// 当前时间（ns）
uint64_t dwt_systime_get_ns(void)
{
    return (dwt_get_cycle64() * 1000000000ULL) / SystemCoreClock;
}

// 当前时间（us）
uint64_t dwt_systime_get_us(void)
{

    return dwt_get_cycle64() / (SystemCoreClock / 1000000ULL);
}

// 当前时间（ms）
uint64_t dwt_systime_get_ms(void)
{

    return dwt_get_cycle64() / (SystemCoreClock / 1000ULL);
}

// 替代 HAL_GetTick()
uint32_t dwt_systime_get_tick(void)
{
    return (uint32_t)dwt_systime_get_ms();
}

void dwt_delay_ms(uint32_t ms)
{
    uint64_t total_ticks = (uint64_t)ms * (SystemCoreClock / 1000ULL);
    uint64_t start_cycle = dwt_get_cycle64();

    while ((dwt_get_cycle64() - start_cycle) < total_ticks)
    {
    }
}

void dwt_delay_us(uint32_t us)
{
    // 将微秒换算成需要等待的 CPU 周期数（乘法远比除法快）
    uint64_t total_ticks = (uint64_t)us * (SystemCoreClock / 1000000ULL);
    uint64_t start_cycle = dwt_get_cycle64();

    while ((dwt_get_cycle64() - start_cycle) < total_ticks)
    {
        // 纯周期比较，没有任何多余的除法运算，1us 极其精准
    }
}
