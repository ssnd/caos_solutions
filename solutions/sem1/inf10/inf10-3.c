//
// Created by Sandu Kiritsa on 02.12.2020.
//

#include <sys/stat.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>


int32_t * ptr;
int32_t file_size;

int32_t GetByteSize(size_t c) {
  if (!c)
    return 0;

  return (c-1)/sizeof(int32_t) + 1;
}

void myalloc_initialize(int fd) {
  struct stat st;
  fstat(fd, &st);
  file_size = st.st_size;
  if (file_size < sizeof(int32_t)) {
    return ;
  }

  file_size = file_size / sizeof(int32_t);

  ptr = mmap(NULL, file_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);

  if (ptr == MAP_FAILED) {
    exit(1);
  }

  memset(ptr, 0, file_size * sizeof(int32_t));
  *ptr = -(int32_t) file_size;
//  *(ptr+1) = 0; // used = 0
}

int32_t FindFreeSpace(size_t size, size_t* free_space_size) {
  if (size == 0) {
    return 0;
  }
  int32_t gap_size = 0;
  for (int i = 0; i < file_size;) {
    int32_t block_size = *(ptr+i);
    if (block_size > 0) {
      gap_size = 0;
    } else if (block_size < 0) {
      block_size*=-1;
      if (block_size + gap_size >= size) {
        *free_space_size = block_size + gap_size;
        return i - gap_size;
      }
      gap_size += block_size;
    }
    i += block_size;
  }

  return -1;
}

void* my_malloc(size_t size) {
  if (size <= 0) {
    return NULL;
  }

  size = GetByteSize(size) + 1;
  size_t free_space_size = 0;
  int32_t free_space_index = FindFreeSpace(size, &free_space_size);
  if (free_space_index == -1) {
    return NULL;
  }

  if (size < free_space_size) {
    *(ptr + free_space_index + size) = -(free_space_size - size);
  }
  *(ptr + free_space_index) = size;
  return ptr + free_space_index + 1;
}

void myalloc_finalize() {
  if (ptr == NULL) {
    exit(1);
  }

  munmap(ptr, file_size);
  ptr = NULL;
}


void my_free(void * ptr_to_free) {
  uint32_t* my_ptr= ptr_to_free;
  if (ptr_to_free == NULL)
    exit(1);

  my_ptr --;

  int32_t block_size = *my_ptr;
  while (my_ptr + block_size < ptr + file_size &&
         *(my_ptr + block_size) < 0) {
    block_size += -*(my_ptr + block_size);
  }
  *(my_ptr) = -block_size;

}