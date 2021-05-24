#include <stdio.h>
#include <stdint.h>


int START_A_UPPER = 10;
int LETTER_COUNT = 26;
int MAX_BITS = 61;

uint16_t GetCharPos(char a)
{
    if (a >= '0' && a <= '9')
        return a - '0';

    if (a >= 'A' && a <= 'Z')
        return (a - 'A') + START_A_UPPER;

    if (a >= 'a' && a <= 'z')
        return (a - 'a') + START_A_UPPER + LETTER_COUNT;

    return 0;
}

uint16_t GetPosChar(uint64_t pos)
{
    if (pos >= 0 && pos <= 9) // it's a number
        return '0' + pos;

    if (pos >= START_A_UPPER && pos < START_A_UPPER + LETTER_COUNT) // uppercase letter
        return 'A' + (pos - START_A_UPPER);

    if (pos >= START_A_UPPER + LETTER_COUNT && pos <= START_A_UPPER + LETTER_COUNT * 2) // lowercase letter
        return 'a' + (pos - (START_A_UPPER + LETTER_COUNT));

    return '-';
}

void DisplayDecodedSymbols(uint64_t x)
{

    for (uint64_t i = 2; i < 64; ++i)
    {
        uint8_t res = (((1ULL << (63 - i)) & x) >> (63 - i));
        if (res)
            printf("%c", GetPosChar(i - 2));
    }
    printf("\n");
}

int main()
{
    uint64_t buff, result, input;
    int16_t ch;
    result = 0;
    input = 0;
    while ((ch = getchar()) != EOF)
    {
        if (ch == '|')
        {
            result |= input;
            input = 0;
            continue;
        }

        if (ch == '&')
        {
            result &= input;
            input = 0;
            continue;
        }

        if (ch == '^')
        {
            result ^= input;
            input = 0;
            continue;
        }

        if (ch == '~')
        {
            result = ~result;
            input = 0;
            continue;
        }

        buff = 1;
        input |= buff << (MAX_BITS - GetCharPos(ch));
    }
    DisplayDecodedSymbols(result);
    return 0;
}