#ifndef __RED_NEC_H
#define __RED_NEC_H
#include <stdint.h>

void Red_Nec_Init(void);

int Red_Nec_Read(uint8_t *pDev, uint8_t *pData);

const char *Red_Nec_CodeToString(uint8_t code);

void Red_Nec_IRQ_Callback(void);
#endif