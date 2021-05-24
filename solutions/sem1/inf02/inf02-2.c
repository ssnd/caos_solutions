
#include <stdint.h>

uint16_t satsum(uint16_t x, uint16_t y) {
    if (((uint16_t)~0U) - y <= x)
        return ((uint16_t)~0U);

    return x+y;
}