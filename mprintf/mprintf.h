#ifndef MPRINTF_H
#define MPRINTF_H

#include "stm32f10x.h"
#define INCLUDE_FLOAT
//#define USE_DEBUG

int printf_(const char *format, ...);
int sprintf_(char *buffer, const char *format, ...);
void set_COM_MAIN(USART_TypeDef * target);

#endif
