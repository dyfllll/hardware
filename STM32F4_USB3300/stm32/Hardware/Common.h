
#ifndef __COMMON_H
#define __COMMON_H
#include <stdint.h>
#include "dwt_systime.h"

#define COM_MIN(x, y) ((x) < (y) ? (x) : (y))
#define COM_MAX(x, y) ((x) > (y) ? (x) : (y))
#define COM_SIGN(x) ((x) >= 0 ? 1 : -1)

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef SIGN
#define SIGN(x) ((x) >= 0 ? 1 : -1)
#endif

#define _2PI 6.28318530718f   // 2π
#define _PI 3.14159265359f    // π
#define _PI_2 1.57079632679f  // π/2
#define _PI_3 1.0471975512f   // π/3
#define _3PI_2 4.71238898038f // 3π/2

float normalize_angle(float angle);
 
// #define COM_NUMBER_SCALE 10000

// #define COMMON_ENTER_CRITICAL UBaseType_t __isrm__ = _COMMON_ENTER_CRITICAL()
// #define COMMON_EXIT_CRITICAL _COMMON_EXIT_CRITICAL(__isrm__)

// static inline UBaseType_t _COMMON_ENTER_CRITICAL(void)
// {
//     UBaseType_t isrm;
//     if (xPortIsInsideInterrupt())
//     {
//         isrm = taskENTER_CRITICAL_FROM_ISR();
//     }
//     else
//     {
//         taskENTER_CRITICAL();
//         isrm = 0;
//     }
//     return isrm;
// }

// static inline void _COMMON_EXIT_CRITICAL(UBaseType_t isrm)
// {
//     if (xPortIsInsideInterrupt())
//     {
//         taskEXIT_CRITICAL_FROM_ISR(isrm);
//     }
//     else
//     {
//         taskEXIT_CRITICAL();
//     }
// }

// static inline void System_Time_Init(void)
// {
//     dwt_systime_init();
// }

// static inline uint64_t System_Time_MS(void)
// {
//     return dwt_systime_get_ms();
// }

// static inline uint64_t System_Time_US(void)
// {
//     return dwt_systime_get_us();
// }

// static inline uint64_t System_Time_NS(void)
// {
//     return dwt_systime_get_ns();
// }

// static inline uint32_t System_Time_Tick(void)
// {
//     return dwt_systime_get_tick();
// }

#endif