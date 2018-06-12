
#include "mprintf.h"

#include <stdarg.h>     // (...) parameter handling
#include <stdlib.h>     //NULL pointer definition

#include "stm32f10x.h"	// only this headerfile is used
//#define assert_param(expr) ((void)0) /*dummy to make the stm32 header work*/

char *SPRINTF_buffer;          //
void putc_strg(char character,char **buffer);
static int vfprintf_(void (*) (char), const char *format, va_list arg); //generic print
void long_itoa (long, int, int, void (*) (char)); //heavily used by printf_()

USART_TypeDef * COM_MAIN = USART1;

void set_COM_MAIN(USART_TypeDef * target){
  COM_MAIN = target;
}

void putc_COM (char c)
{
  if (c == '\n') {
    while ((COM_MAIN->SR & USART_SR_TXE) == 0);  //blocks until previous byte was sent
    COM_MAIN->DR ='\r';
  }
  while ((COM_MAIN->SR & USART_SR_TXE) == 0);  //blocks until previous byte was sent
  COM_MAIN->DR = c;
}

int printf_(const char *format, ...)
{
  va_list arg;
  
  va_start(arg, format);
  vfprintf_((void(*)(char))(&putc_COM), format, arg);
  va_end(arg);

  return 0;
}

int sprintf_(char *buffer, const char *format, ...)
{
  va_list arg;

  SPRINTF_buffer=buffer;	 //Pointer auf einen String in Speicherzelle abspeichern

  va_start(arg, format);
  vfprintf_((void(*)(char))(&putc_strg), format, arg);
  va_end(arg);

  *SPRINTF_buffer ='\0';             // append end of string

  return 0;
}

/**
 * @def debug(format...)
 * @brief prints the timestamp, file name, line number, printf-formated @a format string and the
 * optional parameters to stdout
 *
 * The output looks like this:<br>
 * <pre>
 * 12345     filename.c[123]: format string
 * ^    ^    ^          ^
 * |    |    |          line number
 * |    |    +--------- file name
 * |    +-------------- tab character
 * +------------------- timestamp (ms since reset)
 * </pre>
 *
 * */
#ifdef USE_DEBUG
	#define debug(format,...) {\
		printf_("%ul\t%s[%i]: ", millisec, __FILE__, __LINE__); /* print file name and line number */\
		printf_(format, ## __VA_ARGS__);               /* print format string and args */\
		printf_("\n"); \
	}
#else
	#define debug(format,...) ((void)0)
#endif /* USE_DEBUG */



/*
+=============================================================================+
| local functions
+=============================================================================+
*/
// putc_strg() is the putc()function for sprintf_()
void putc_strg(char character,char **buffer)
{
  *SPRINTF_buffer = (char)character;	// just add the character to buffer
  SPRINTF_buffer++;

}

/*--------------------------------------------------------------------------------+
 * vfprintf_()
 * Prints a string to stream. Supports %s, %c, %d, %ld %ul %02d %i %x  %lud  and %%
 *     - partly supported: long long, float (%l %f, %F, %2.2f)
 *     - not supported: double float and exponent (%e %g %p %o \t)
 *--------------------------------------------------------------------------------+
*/
static int vfprintf_(void (*putc)(char), const char* str,  va_list arp) {
  int d, r, w, s, l;  //d=char, r = radix, w = width, s=zeros, l=long
  char *c;            // for the while loop only

#ifdef INCLUDE_FLOAT
  float f;
  long int m, w2;
#endif

  while ((d = *str++) != 0) {
    if (d != '%') {
      (*putc)(d);
      continue;
    }
    d = *str++;
    w = r = s = l = 0;
    if (d == '%') {
      (*putc)(d);
      d = *str++;
    }
    if (d == '0') {
      d = *str++; s = 1;  //padd with zeros
    }
    while ((d >= '0')&&(d <= '9')) {
      w += w * 10 + (d - '0');
      d = *str++;
    }
    if (s) w = -w;      //padd with zeros if negative
    
#ifdef INCLUDE_FLOAT
    w2 = 0;
    if (d == '.')
      d = *str++;
    while ((d >= '0')&&(d <= '9')) {
      w2 += w2 * 10 + (d - '0');
      d = *str++;
    }
#endif
    
    if (d == 's') {
      c = va_arg(arp, char*);
      while (*c)
	(*putc)(*(c++));
      continue;
    }
    if (d == 'c') {
      (*putc)((char)va_arg(arp, int));
      continue;
    }
    if (d == 'u') {     // %ul
      r = 10;
      d = *str++;
    }
    if (d == 'l') {     // long =32bit
      l = 1;
      if (r==0) r = -10;
      d = *str++;
    }
    //		if (!d) break;
    if (d == 'u') r = 10;//     %lu,    %llu
    else if (d == 'd' || d == 'i') {if (r==0) r = -10;}  //can be 16 or 32bit int
    else if (d == 'X' || d == 'x') r = 16;               // 'x' added by mthomas
    else if (d == 'b') r = 2;
    else str--;                                         // normal character
    
#ifdef INCLUDE_FLOAT
    if (d == 'f' || d == 'F') {
      f=va_arg(arp, double);
      if (f>0) {
	r=10;
	m=(int)f;
      }
      else {
	r=-10;
	f=-f;
	m=(int)(f);
      }
      long_itoa(m, r, w, (putc));
      f=f-m; m=f*(10^w2); w2=-w2;
      long_itoa(m, r, w2, (putc));
      l=3; //do not continue with long
    }
#endif
    
    if (!r) continue;  //
    if (l==0) {
      if (r > 0){      //unsigned
	long_itoa((unsigned long)va_arg(arp, int), r, w, (putc)); //needed for 16bit int, no harm to 32bit int
      }
      else            //signed
	long_itoa((long)va_arg(arp, int), r, w, (putc));
    } else if (l==1){  // long =32bit
      long_itoa((long)va_arg(arp, long), r, w, (putc));        //no matter if signed or unsigned
    }
  }
  
  return 0;
}


void long_itoa (long val, int radix, int len, void (*putc) (char))
{
  char c, sgn = 0, pad = ' ';
  char s[20];
  int  i = 0;
  
  if (radix < 0) {
    radix = -radix;
    if (val < 0) {
      val = -val;
      sgn = '-';
    }
  }
  if (len < 0) {
    len = -len;
    pad = '0';
  }
  if (len > 20) return;
  do {
    c = (char)((unsigned long)val % radix); //cast!
    if (c >= 10) c += ('A'-10); //ABCDEF
    else c += '0';            //0123456789
    s[i++] = c;
    val = (unsigned long)val /radix; //cast!
  } while (val);
  if (sgn) s[i++] = sgn;
  while (i < len)
    s[i++] = pad;
  do
    (*putc)(s[--i]);
  while (i);
}
