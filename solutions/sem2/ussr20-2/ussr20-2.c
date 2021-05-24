#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <semaphore.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

volatile sig_atomic_t smoking = 0;
volatile sig_atomic_t sigterm_recv = 0;
pid_t * procs;


void cleanup() {
  sem_unlink("/tobacco");
  sem_unlink("/matches");
  sem_unlink("/bartender");
  sem_unlink("/paper");

  free(procs);
}

void SIGTERMHandler() {
  sigterm_recv = 1;
  if (smoking == 0) {
    cleanup();
    _exit(0);
  }
}

struct client_desc {
    char name;
    sem_t *semaphore;
    sem_t *bartender;
};


void spawn_client(pid_t * pid, struct client_desc * desc) {
  char name = desc->name;
  sem_t* semaphore = desc->semaphore;
  sem_t* bartender = desc->bartender;

  *pid = fork();
  if (*pid == 0) { // child
    while (sigterm_recv == 0) {
      sem_post(bartender);
      sem_wait(semaphore);

      smoking = 1;
      printf("%c\n", name);
      fflush(stdout);
      smoking = 0;
    }
  }
}



int main() {
  struct sigaction sigterm_handler;
  memset(&sigterm_handler, 0, sizeof(sigterm_handler));
  sigterm_handler.sa_handler = SIGTERMHandler;
  sigterm_handler.sa_flags = SA_RESTART;
  sigaction(SIGTERM, &sigterm_handler, NULL);

  sem_t * sem_bartender = sem_open("/bartender", O_CREAT, 0777, 0);

  procs = malloc(sizeof(pid_t) * 3);

  struct client_desc tobacco;
  tobacco.name = 'T';
  tobacco.semaphore =  sem_open("/tobacco", O_CREAT, 0777, 0);
  tobacco.bartender = sem_bartender;
  spawn_client(&procs[0], &tobacco);

  struct client_desc paper;
  paper.name = 'P';
  paper.semaphore = sem_open("/paper", O_CREAT, 0777, 0);
  paper.bartender = sem_bartender;
  spawn_client(&procs[1], &paper);

  struct client_desc matches;
  matches.name = 'M';
  matches.semaphore = sem_open("/matches", O_CREAT, 0777, 0);
  matches.bartender = sem_bartender;
  spawn_client(&procs[2], &matches);

  for (size_t i = 0; i < 3; ++i)
    sem_wait(sem_bartender);

  for (size_t i = 0; i < 3; ++i)
    sem_post(sem_bartender);
  char ch;

  while (scanf("%c", &ch) != -1) {
    switch (ch)  {
      case 't':
        sem_post(tobacco.semaphore);
        sem_wait(sem_bartender);
        break;

      case 'p':
        sem_post(paper.semaphore);
        sem_wait(sem_bartender);
        break;

      case 'm':
        sem_post(matches.semaphore);
        sem_wait(sem_bartender);
        break;

      default:
        break;
    }
  }

  for (size_t i = 0; i < 3; ++i)
    kill(procs[i], SIGTERM);

  for (size_t i = 0; i < 3; ++i)
    waitpid(procs[i], NULL, 0);


  cleanup(procs);

  return 0;

}