#include <stdio.h>

int fprintf(FILE *stream, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    return vfprintf(stream, format, ap);
}
