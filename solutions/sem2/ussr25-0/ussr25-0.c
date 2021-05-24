#include <stdio.h>

#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* memory = malloc(2048);
    memset(memory, 0, 2048);
    size_t i = 0;
    while (scanf("%c", &memory[i]) != EOF) {
        ++i;
    }
    unsigned char hashed[SHA512_DIGEST_LENGTH];
    SHA512(memory, strlen(memory), hashed);
    printf("0x");

    for (i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
        printf("%02x", hashed[i]);
    }
    free(memory);

    return 0;
}