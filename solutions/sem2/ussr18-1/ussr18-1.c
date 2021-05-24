#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

//#define DEBUG 1

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif


typedef uint64_t int_;

static void* thread_func(void * arg) {
    dbg("thread working\n");
    int_ result =0 ;
    int_ current;
//    result = 0;

    while (scanf("%ld ", &current) != -1) {
        result += current;
        dbg("%ld\n", result);
    }
    return (void*)result;
}

void spawn_threads(size_t n, pthread_t * threads) {
    dbg("spawning threads\n");
    for (size_t i = 0; i < n; ++i) {
        pthread_create(&threads[i], NULL, thread_func, 0);
    }
}

int_ join_threads(size_t n, pthread_t * threads) {
    int_ result = 0;

    for (size_t i = 0; i < n; ++i) {
        int_ temp_result;
        pthread_join(threads[i], (void*)&temp_result);
        dbg("temp result = %ld\n", temp_result);
        result+=temp_result;
    }

    return result;
}

int main(int argc, char *argv[]) {

    #ifdef DEBUG
        size_t thread_count = 4;
    #else
        size_t thread_count = strtol(argv[1], NULL, 10);
    #endif

    pthread_t *threads = malloc(sizeof(pthread_t) * thread_count);
    spawn_threads(thread_count, threads);
    int_ result = join_threads(thread_count, threads);

    printf("%ld", result);
    return 0;
}