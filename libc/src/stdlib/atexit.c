#include <stdlib.h>
#include <stddef.h>

void (* _atexit_tab[ATEXIT_MAX])(void);

int atexit(void (* func)(void))
{
    int i, res = -1;

    for (i = 0; i < ATEXIT_MAX; i++) {
        if (_atexit_tab[i] == NULL) {
            _atexit_tab[i] = func;
            res = 0;
            break;
        }
    }
    return res;
}
