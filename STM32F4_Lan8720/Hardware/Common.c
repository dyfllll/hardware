#include "Common.h"



float normalize_angle(float angle)
{
    float a = fmodf(angle, _2PI); // 取余运算可以用于归一化，列出特殊值例子算便知
    return a >= 0 ? a : (a + _2PI);
    // 三目运算符。格式：condition ? expr1 : expr2
    // 其中，condition 是要求值的条件表达式，如果条件成立，则返回 expr1 的值，否则返回 expr2 的值。可以将三目运算符视为 if-else 语句的简化形式。
    // fmod 函数的余数的符号与除数相同。因此，当 angle 的值为负数时，余数的符号将与 _2PI 的符号相反。也就是说，如果 angle 的值小于 0 且 _2PI 的值为正数，则 fmod(angle, _2PI) 的余数将为负数。
    // 例如，当 angle 的值为 -PI/2，_2PI 的值为 2PI 时，fmod(angle, _2PI) 将返回一个负数。在这种情况下，可以通过将负数的余数加上 _2PI 来将角度归一化到 [0, 2PI] 的范围内，以确保角度的值始终为正数。
}


// void i2c_scan(I2C_HandleTypeDef *hi2c)
// {

//   uint8_t u8i = 1;
//   uint8_t data = 0x00;
//   uint8_t err_code = 0;

//   printf("scan i2c bus:\r\n");
//   for (u8i = 1; u8i < 127; u8i++)
//   {
//     err_code = HAL_I2C_Master_Transmit(hi2c, u8i << 1, &data, 1, 200);
//     if (err_code == 0)
//     {
//       printf("found slave device address :0x%02x\r\n", u8i);
//       HAL_Delay(20);
//     }
//     else
//     {
//       // printf("no found slave device address :0x%02x\r\n", u8i);
//     }
//     HAL_Delay(20);
//   }
//   printf("scan i2c bus finish\r\n");
// }
