#ifndef __COMMON_H
#define __COMMON_H
#include <stdint.h>
#include "dwt_systime.h"

#define COM_MIN(x, y) ((x) < (y) ? (x) : (y))
#define COM_MAX(x, y) ((x) > (y) ? (x) : (y))

void udelay(int us);

void mdelay(int ms);

uint32_t system_get_ms(void);

uint64_t system_get_us(void);

uint64_t system_get_ns(void);

#endif