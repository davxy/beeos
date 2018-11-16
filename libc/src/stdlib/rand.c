#include <stdlib.h>

/*
 * The `_srand' rank must be at least 'unsigned int' and must be able
 * to represent the value 'RAND_MAX'.
 */
static unsigned int _srand = 1;

int rand(void)
{
    _srand = _srand * 12345 + 123;
    return _srand % ((unsigned int) RAND_MAX + 1);
}

void srand(unsigned int seed)
{
    _srand = seed;
}
