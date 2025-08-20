#include "dwt_systime.h"

static uint32_t last_cycle = 0;
static uint64_t high_cycle = 0;

void DWT_SysTime_Init(void) {
    // 启用 DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// 获取 64 位 cycle，处理溢出
static inline uint64_t DWT_GetCycle64(void) {
    uint32_t now = DWT->CYCCNT;
    if (now < last_cycle) {
        high_cycle += (1ULL << 32);  // 溢出补偿
    }
    last_cycle = now;
    return high_cycle | now;
}

// 当前时间（ns）
uint64_t DWT_SysTime_Get_ns(void) {
    uint64_t cycles = DWT_GetCycle64();
    return (cycles * 1000000000ULL) / SystemCoreClock;
}

// 当前时间（us）
uint64_t DWT_SysTime_Get_us(void) {
    uint64_t cycles = DWT_GetCycle64();
    return cycles / (SystemCoreClock / 1000000ULL);
}

// 当前时间（ms）
uint64_t DWT_SysTime_Get_ms(void) {
    uint64_t cycles = DWT_GetCycle64();
    return cycles / (SystemCoreClock / 1000ULL);
}

// 替代 HAL_GetTick()
uint32_t DWT_SysTime_GetTick(void) {
    return (uint32_t)DWT_SysTime_Get_ms();
}