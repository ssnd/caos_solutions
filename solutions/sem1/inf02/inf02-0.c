#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    double x;
    int64_t y,z;
    scanf("%lg", &x);
    scanf("%lx", &y);
    z = strtoll(argv[1], NULL, 27);
    printf("%.3f", x + (double) y + z);
    return 0;
}