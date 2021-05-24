#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void swap(int f, int index1, int index2) {
  int32_t buff1,buff2;
  lseek(f, index1*sizeof(int32_t), SEEK_SET);
  read(f, &buff1, sizeof(int32_t));
  lseek(f, index2*sizeof(int32_t), SEEK_SET);
  read(f, &buff2, sizeof(int32_t));
  lseek(f,index1*sizeof(int32_t),SEEK_SET);
  write(f, &buff2, sizeof(int32_t));
  lseek(f, index2*sizeof(int32_t), SEEK_SET);
  write(f, &buff1, sizeof(int32_t));
}

int32_t read_index(int f, int32_t index) {
  int32_t value;
  lseek(f, index*4, SEEK_SET);
  read(f, &value, sizeof(int32_t));
  return value;
}

void assign_index(int f, int32_t index, int32_t value) {
  lseek(f, index*sizeof(int32_t), SEEK_SET);
  write(f, &value, sizeof(int32_t));
}


void merge(int f, int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    int temp_l = open("../t_l", O_RDWR | O_CREAT, 0640);
    int temp_r = open("../t_r", O_RDWR | O_CREAT, 0640);

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++) {
      int32_t t = read_index(f, l+i);
      assign_index(temp_l, i, t);
    }
    for (j = 0; j < n2; j++) {
      int32_t t = read_index(f, m + 1 + j);
      assign_index(temp_r, j, t);
    }


    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (read_index(temp_l, i) <= read_index(temp_r, j)) {
          assign_index(f, k, read_index(temp_l, i));
            i++;
        }
        else {
          assign_index(f, k, read_index(temp_r, j));
            j++;
        }
        k++;
    }

    while (i < n1) {
        assign_index(f, k, read_index(temp_l, i));
        i++;
        k++;
    }
    while (j < n2) {
        assign_index(f, k, read_index(temp_r, j));
        j++;
        k++;
    }

    close(temp_l);
    close(temp_r);
}

void mergeSort(int f, int l, int r)
{
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSort(f, l, m);
        mergeSort(f, m + 1, r);

        merge(f, l, m, r);
    }
}



int main(int argc, char* argv[]) {
  int f = open(argv[1], O_RDWR);

//  srand(3.1415);
//  int32_t mv = 0;
//  int32_t a;
//  for (size_t i = 0; i < 100; ++i) {
//    a = rand() % 10000 - 5000;
//    printf("%d ", a );
//    lseek(f, mv, SEEK_SET);
//    write(f, &a, sizeof(int32_t));
//    mv+=sizeof(int32_t);
//  }

  off_t end_pos = lseek(f, 0, SEEK_END);
  off_t beg_pos = lseek(f, 0, SEEK_SET);

  int32_t n = (end_pos - beg_pos)/sizeof(int32_t);

//
//  printf("\nread:\n");
//  mv = 0;
//  for (size_t i = 0 ; i < 100; ++i) {
//    lseek(f, mv, SEEK_SET);
//    read(f, &a, sizeof(int32_t));
//    printf("%d ", a);
//    mv+=4;
//  }


  mergeSort(f, 0, n-1);
//  printf("\nsorted:\n");
//  mv = 0;
//  for (size_t i =0 ; i < 100; ++i) {
//    lseek(f, mv, SEEK_SET);
//    read(f, &a, sizeof(int32_t));
//    printf("%d ", a);
//    mv+=4;
//  }

  close(f);
}