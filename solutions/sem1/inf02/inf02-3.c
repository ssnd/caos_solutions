#include <stdio.h>

int main(int argc, char *argv[])
{
    for (size_t i = 1; i < argc; ++i)
    {
        size_t j = 0;
        size_t hex_letters = 0;
        while (argv[i][j] == '0' || argv[i][j] == 'x')
            ++j;

        while (argv[i][j] != '\0')
        {
            ++hex_letters;
            ++j;
        }
        size_t res = hex_letters / 2 + hex_letters % 2;

        printf("%ld ", (res == 0) ? 1 : res);
    }
    printf("\n");
}