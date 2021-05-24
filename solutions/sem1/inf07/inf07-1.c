#include <sys/syscall.h>

long syscall(long number, ...);

void _start()
{

    while (1) {
        char m[4096];

        int total_chars = syscall(SYS_read, 0, m, sizeof(m));
        if (total_chars == 0)
            break;

        syscall(SYS_write, 1, m, total_chars);
    }

    syscall(SYS_exit, 0);
}