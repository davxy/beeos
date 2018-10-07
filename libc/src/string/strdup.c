#include <string.h>
#include <stdlib.h>
#include <errno.h>

char *strdup(const char *s)
{
    size_t size;
    char *s2;

    /* Get string size: must be added 1, to count the termination null. */
    size = strlen(s) + 1;
    s2 = malloc(size);
    if (s2 == NULL)
        return NULL;
    memcpy(s2, s, size);
    return s2;
}
