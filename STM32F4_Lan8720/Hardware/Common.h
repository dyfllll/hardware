
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
 
#endif