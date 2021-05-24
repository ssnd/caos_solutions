#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <windows.h>
#include <fileapi.h>



void stdout_print(int value) {
  HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);
  if(stdout && stdout != INVALID_HANDLE_VALUE) {
    if (value < 0) {
      WriteFile(stdout, "-", 1, NULL, NULL);
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
      int output = '0' + (value / div) % kTen;
      WriteFile(stdout, &output, 1, NULL, NULL);
      div /= kTen;
    }

    WriteFile(stdout, " ", 1, NULL, NULL);
  }
}


int main(int argc, char* argv[]) {
  int value = 0;
  int8_t ex_code = 0;
  int read_bytes;
  LARGE_INTEGER weird_windows_value;
  uint32_t next_pointer;
  HANDLE input_file = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (input_file == INVALID_HANDLE_VALUE) {
    ex_code = 1;
    goto exit;
  }

  while (1) {
//    read_value_status = read(input_file, &value, sizeof(uint32_t));
//    read_pointer_status = read(input_file, &next_pointer, sizeof(uint32_t));


    if (!ReadFile(input_file, &value, sizeof(uint32_t), &read_bytes, NULL) || read_bytes == 0)
      break;

    stdout_print(value);

    ReadFile(input_file, &next_pointer, sizeof(uint32_t), &read_bytes, NULL);
    if (next_pointer == 0)
      break;

    weird_windows_value.QuadPart = next_pointer;
    SetFilePointerEx(input_file, weird_windows_value, NULL, FILE_BEGIN);
//    lseek(input_file, next_pointer, SEEK_SET);
  }


  exit:
    CloseHandle(input_file);
    return ex_code;

}