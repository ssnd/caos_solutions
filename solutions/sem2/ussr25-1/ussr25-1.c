#include <openssl/crypto.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <limits.h>
#include <unistd.h>


int main (int argc, char * argv[]) {
  char * pass = argv[1];

  unsigned char salt[8];
  char buff[4096];
  char output[8192];

  read(0, salt, 8);
  read(0, salt, 8);

  unsigned char key[4096];
  unsigned char input_vector[4096];

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt, pass, strlen(pass),1, key, input_vector);
  EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key,input_vector);

  int read_c = 0;
  int written = 0;

  while ((read_c = read(0, buff, 4096))!=0) {
    EVP_DecryptUpdate(ctx, output, &written, buff, read_c);
    if (written != 0) {
      write(1, output, written);
    }
  }

  EVP_DecryptFinal(ctx, output, &written);

  if (written != 0) {
    write(1, output, written);
  }
}
