#ifndef MPRINTF_H
#define MPRINTF_H

#define INCLUDE_FLOAT
//#define USE_DEBUG

#if defined(STM32F10X_MD)
#include "stm32f10x.h"
#elif defined(STM32F4XX)
#include "stm32f4xx.h"
#elif defined(STM32F030)
#include "stm32f0xx.h"
#else
#error "device type missing!"
#endif

int printf_(const char *format, ...);
int sprintf_(char *buffer, const char *format, ...);
void set_COM_MAIN(USART_TypeDef * target);

#endif
