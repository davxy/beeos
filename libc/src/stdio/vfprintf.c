#include "FILE.h"
#include <stdio.h>
#include <unistd.h>

int vfprintf(FILE *stream, const char *format, va_list ap)
{
    int n, w, left;
    char buf[BUFSIZ];
    char *p = buf;

    if ((n = vsnprintf(buf, BUFSIZ, format, ap)) < 0)
        return n;
    left = n;
    while (left > 0) {
        w = write(stream->fd, p, left);
        if (w < 0)
            break;
        left -= w;
        p += w;
    }
    return n - left;
}
