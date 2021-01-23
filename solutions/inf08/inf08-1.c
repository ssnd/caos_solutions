#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>


void stdout_print(int value) {
  if (value < 0) {
    write(1, "-", 1);
    value *= -1;
  }

  int kTen = 10;
  int div = 1;

  int value2 = value;

  while (value2 != 0) {
    div *= kTen;
    value2 /= kTen;
  }

  div /= kTen;

  while (div) {
    int output = '0' + (value/div)%kTen;
    write(1, &output, 1);
    div/=kTen;
  }

  write(1, " ", 1);
}


int main(int argc, char* argv[]) {
  int value = 0;
  int8_t ex_code = 0;
  uint32_t next_pointer;
  int8_t read_value_status, read_pointer_status;
  int8_t input_file = open(argv[1], O_RDONLY);
  if (input_file == -1) {
    ex_code = 1;
    goto exit;
  }

  while (1) {
    read_value_status = read(input_file, &value, sizeof(uint32_t));
    read_pointer_status = read(input_file, &next_pointer, sizeof(uint32_t));

    if (read_value_status == 0)
      break;

    if (read_pointer_status < 0 || read_value_status < 0) {
      ex_code = 1;
      goto exit;
    }

    stdout_print(value);
    if (next_pointer == 0)
      break;

    lseek(input_file, next_pointer, SEEK_SET);
  }


  exit:
    close (input_file);
    return ex_code;

}
