#include "driver_irq.h"
#include "stm32f1xx_hal.h"
#include "Red_Nec.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    switch (GPIO_Pin)
    {
    case GPIO_PIN_8:
    {
        Red_Nec_IRQ_Callback();
        break;
    }

    default:
    {
        break;
    }
    }
}