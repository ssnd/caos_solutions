#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h> 
#include <assert.h>
#include <time.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <semaphore.h>




typedef double (*function_t)(double);

double *pmap_process(function_t func, const double *in, size_t count) {

  int shm_fd = shm_open("/shm_file", O_CREAT | O_RDWR, 0777);
  ftruncate(shm_fd, count);
  double *output = mmap(0, count, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  int proc_count = get_nprocs();

  void* semaphore = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  sem_init(semaphore,1,0);


  for (size_t i = 0; i < proc_count; ++i) {
    pid_t proc_pid = fork();
    if (0 == proc_pid) { // is child
      size_t from = i * (count / proc_count);
      size_t to = from + (count / proc_count);

      if (i == proc_count - 1) {
        to += count % proc_count;
      }

      for (size_t res_index = from; res_index < to; ++res_index) {
        output[res_index] = func(in[res_index]);
      }

      sem_post(semaphore);
      exit(0);
    }
  }

  for (size_t i = 0; i < proc_count; ++i) {
    sem_wait(semaphore);
  }
  for (size_t i = 0; i < proc_count; ++i) {
    wait(NULL);
  }

  sem_destroy(semaphore);
  munmap(semaphore,sizeof(sem_t));
  return output;
}

void pmap_free(double *ptr, size_t sz) {
  munmap(ptr, sz);
  shm_unlink("/shm_file");
}