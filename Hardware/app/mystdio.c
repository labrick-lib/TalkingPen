#include <stdarg.h>
#include <stdio.h>
#include "new_type.h"
#include "DrvUART.h" 
#define PRINTF_BUF_LEN  128

#if 0
//be sure the length of the string is not exceeding 128 bytes.
int printf(const char * format, ...)
{
	va_list ap;
	int		count;
	char	buf[PRINTF_BUF_LEN];
	
	va_start(ap, format);

    count = sprintf(buf, format, ap);

    if(count > sizeof(buf))
        
    
	DrvUART_Write(UART_PORT0, (T_U8*)buf, count);

	return count;
}
#endif

T_VOID akerror(T_U8 *s, T_U32 n, T_BOOL newline)
{
 	printf("%s0x%x%s", s, n, newline ? "\n":"");
}

