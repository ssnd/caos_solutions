#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif

struct cv_mut {
  int64_t A;
  int64_t B;
  int64_t N;
  uint64_t value;
  pthread_cond_t *condvar;
  pthread_cond_t *condvar2;
  int8_t consumer_ready;
  int8_t producer_ready;
  pthread_mutex_t *mutex;
};

int8_t is_prime(int64_t x) {
  for (size_t i = 2; i * i <= x; ++i) {
    if (x % i == 0) {
      return 0;
    }
  }
  return 1;
}



static void *thread_func(void *arg) {
  struct cv_mut *a = (struct cv_mut *) arg;
  pthread_mutex_lock(a->mutex);
  size_t counter = 0;
  int64_t current = a->A;
  while (counter < a->N) {
    while (!is_prime(current)) {
      current += 1;
    }
    a->value = current;

    a->consumer_ready = 1;
    pthread_cond_signal(a->condvar);

    while (!a->producer_ready) {
      pthread_cond_wait(a->condvar2, a->mutex);
    }
    a->producer_ready = 0;

    counter++;
    current++;
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  struct cv_mut pair;

  int64_t A = strtol(argv[1], NULL, 10);
  int64_t B = strtol(argv[2], NULL, 10);
  int64_t N = strtol(argv[3], NULL, 10);

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
  pthread_cond_t condvar2 = PTHREAD_COND_INITIALIZER;
  pthread_t thread;

  pair.A = A;
  pair.B = B;
  pair.N = N;
  pair.condvar = &condvar;
  pair.condvar2 = &condvar2;
  pair.consumer_ready = 0;
  pair.producer_ready = 0;
  pair.mutex = &mutex;

  pthread_mutex_lock(&mutex);
  pthread_create(&thread, NULL, thread_func, (void *) &pair);
  size_t recv_n = 0;
  while (recv_n < N) {

    while (!pair.consumer_ready) {
      pthread_cond_wait(&condvar, &mutex);
    }
    pair.consumer_ready = 0;

    printf("%lu\n", pair.value);
    recv_n++;

    pair.producer_ready = 1;
    pthread_cond_signal(&condvar2);
  }

  pthread_mutex_unlock(&mutex);
  pthread_join(thread, NULL);

  return 0;
}