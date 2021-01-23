#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static char program_text[4096] = "#include <stdio.h>\n"
                                 "int main() {\n"
                                 "int a = (%s);\n"
                                 "printf(\"%%d\", a);\n"
                                 "return 0;\n"
                                 "}\0";

void WriteToFile(char* execstr)
{
    int fd = open("temp_prog.c", O_TRUNC | O_RDWR | O_CREAT, 0664);
    write(fd, execstr, strlen(execstr));
    close(fd);
}

void CompileAndRun()
{
    pid_t p = fork();
    if (p == 0) {
        execlp("gcc", "gcc", "temp_prog.c", "-o", "temp_prog.out", NULL);
        exit(0);
    } else {
        int status;
        waitpid(p, &status, 0);
    }

    p = fork();
    if (p == 0) {
        execlp("./temp_prog.out", "./temp_prog.out", NULL);
        exit(0);
    } else {
        int status;
        waitpid(p, &status, 0);
    }
    remove("temp_prog.c");
    remove("temp_prog.out");
}

int main(int argc, char* argv[])
{

    char buffer[4096];
    char execstr[4096];
    fgets(buffer, sizeof(buffer), stdin);

    if (strlen(buffer) == 0) {
        exit(0);
    };

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';

    snprintf(execstr, sizeof(execstr), program_text, buffer);
    WriteToFile(execstr);
    CompileAndRun();
    return 0;
}
