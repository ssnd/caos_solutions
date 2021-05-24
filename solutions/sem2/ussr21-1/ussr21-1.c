#include <sys/mman.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory.h>
#include <dlfcn.h>

//#define DEBUG 1

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif

// don't want my code to yell at me
#define null NULL

typedef struct {
  sem_t request_ready;
  sem_t response_ready;
  char func_name[20];
  double value;
  double result;
} shared_data_t;

void * dl_open_shared_lib(const char * path) {

  void * lib = dlopen(path, RTLD_NOW);
  if(!lib) {
    fprintf(stderr, "dlopen err: %s\n", dlerror());
  }

  return lib;
}

typedef double(*func_t)(double);
// exec the function with double(*f)(double) signature
// store the result into the value and result variables
// of the given `shared_data_t` struct
void dl_exec_func(void * lib, shared_data_t * data) {

  void * entry = dlsym(lib, data->func_name);
  if (!entry) {
    fprintf(stderr, "dlsym err: %s\n",dlerror());
  }

  func_t func = entry;
  double result = func(data->value);
  data->result = result;
}

int main_loop(void * lib, shared_data_t* data) {
  dbg("request_ready\n");
  sem_wait(&data->request_ready);
  dbg("exec_func\n");
  if (strlen((const char * )&data->func_name) == 0) {
    return 0;
  }

  dl_exec_func(lib, data);
  dbg("response_ready\n");
  sem_post(&data->response_ready);
  return 1;
}

int main(int argc, char * argv[]) {
  const char * shm_seg_name = "/BEPTYXA_OT_DEDA";
  int sh = shm_open(shm_seg_name, O_CREAT | O_RDWR, 0755);

  ftruncate(sh, sizeof(shared_data_t));
  shared_data_t *payload = mmap(0, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, sh, 0);

  sem_init(&payload->response_ready, 1, 0);
  sem_init(&payload->request_ready, 1, 0);

  memset(payload->func_name, 0, 20);
  payload->value   = 0;
  payload->result  = 0;

  printf("%s\n", shm_seg_name);
  fflush(stdout);

#ifdef DEBUG
  void * lib = dl_open_shared_lib("./libmylib.so");
#else
  void * lib = dl_open_shared_lib(argv[1]);
#endif

  while(main_loop(lib, payload)){
    dbg("main_loop\n");
  }

  dbg("done, cleaning up\n");
#ifndef DEBUG
  dlclose(dl_open_shared_lib(argv[1]));
#endif
  shm_unlink(shm_seg_name);
  sem_destroy(&payload->response_ready);
  sem_destroy(&payload->request_ready);
  munmap(payload, sizeof(shared_data_t));

  return 0;
}
