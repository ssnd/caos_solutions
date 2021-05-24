#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char buffer[4096];
    char execstr[4096];

    fgets(buffer, sizeof(buffer), stdin);

    if (strlen(buffer) == 0) {
        return 0;
    };
    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';

    snprintf(execstr, sizeof(execstr), "a = %s; print(a)", buffer);
    execlp("python3", "python3", "-c", execstr, NULL);
    perror("pyexec");
    exit(1);
}