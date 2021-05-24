//
// Created by Sandu Kiritsa on 05/10/2020.
//
#include <stdio.h>
#include <stdint.h>

int64_t FOUR_BITS = 0xf0;
int64_t THREE_BITS = 0xe0;
int64_t TWO_BITS = 0xc0;
int UNICODE_MASK = 0x80;

int GetBytesLength(int64_t ch)
{
    if ((ch & FOUR_BITS) == FOUR_BITS)
        return 4;

    if ((ch & THREE_BITS) == THREE_BITS)
        return 3;

    if ((ch & TWO_BITS) == TWO_BITS)
        return 2;

    return 0;
}

int main()
{
    int ch, add_char;
    size_t ascii_count = 0;
    size_t utf_count = 0;
    while ((ch = getchar()) != EOF)
    {
        if ((ch & (1 << 7)) == 0)
        {
            ++ascii_count;
            continue;
        }

        size_t additional_chars = GetBytesLength(ch);
        if (additional_chars == 0)
            goto err;



        for (size_t i = 0; i < additional_chars - 1; ++i)
        {
            add_char = getchar();
            if (add_char == '\0')
                goto err;
            if ((add_char & UNICODE_MASK) != UNICODE_MASK)
                goto err;
        }
        ++utf_count;
    }

    printf("%ld %ld", ascii_count, utf_count);
    return 0;

err:
{
    printf("%ld %ld", ascii_count, utf_count);
    return 1;
}
}