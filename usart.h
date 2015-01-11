#ifndef __USART_H
#define __USART_H

#include "stm32f429i_discovery.h"

void RCC_Configuration(void);
void GPIO_Configuration(void);
void USART1_Configuration(void);
void USART1_puts(char* s);
void USART1_putsHex(unsigned int num);
#endif 