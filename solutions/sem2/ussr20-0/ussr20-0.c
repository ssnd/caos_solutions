#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>



int main() {
  char sem_nm[4096];
  char shm_nm[4096];
  int n;


  scanf("%s", sem_nm);
  scanf("%s", shm_nm);
  scanf("%d", &n);

  sem_t * semaphore = sem_open(sem_nm, 0);
  int count = n*sizeof(int);


  int shm_fd = shm_open(shm_nm, O_RDWR|O_CREAT, 0664);
  int * arr = mmap(NULL, count, PROT_READ, MAP_SHARED, shm_fd, 0);

  close(shm_fd);

  for (size_t i = 0; i < n; ++i) {
    sem_wait(semaphore);
    printf("%d ", arr[i]);
    sem_post(semaphore);
  }

  sem_close(semaphore);
  sem_unlink(sem_nm);
  munmap(arr, count);

  return 0;

}
