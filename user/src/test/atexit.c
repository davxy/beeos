#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void cb1(void)
{
    printf("callback1\n");
}

void cb2(void)
{
    printf("callback2\n");
}

void cb3(void)
{
    printf("callback3\n");
}

int main(void)
{
    char * const argv[] = { "/bin/ls", NULL };
    printf("Registering callbacks\n");
    atexit(cb1);
    atexit(cb2);
    atexit(cb3);

    if (fork() == 0) {
        /* child */
        if (execve(argv[0], argv, NULL) < 0)
            printf("execve error\n");
    } else {
        /* parent */
    }
    return 0;
}
