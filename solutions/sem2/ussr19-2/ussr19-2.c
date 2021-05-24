#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>


//#define DEBUG 1

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif

// don't want my code to yell at me
#define null NULL

typedef int64_t linked_list_value_t;

typedef struct linked_list_node {
  struct linked_list_node * next;
  linked_list_value_t value;
} linked_list_node_t;

typedef struct linked_list {
  _Atomic (linked_list_node_t *) head;
  _Atomic( linked_list_node_t *) tail;
} linked_list_t;


linked_list_node_t * linked_list_create_node(linked_list_value_t value) {
  linked_list_node_t * new_node =  malloc(sizeof(linked_list_node_t));
  new_node->value = value;
  new_node->next = null;
  return new_node;
}

linked_list_t * linked_list_init() {
  // initializing the head node
  dbg("initializing a new linked list\n");
  linked_list_t * list = malloc(sizeof(linked_list_t));
  linked_list_node_t * head = linked_list_create_node(-1);
  atomic_store(&(list->head), head);
  atomic_store(&(list->tail), head);
  return list;
}

void linked_list_push(linked_list_t * list, linked_list_value_t value) {
  linked_list_node_t * tail = atomic_load(&(list->tail));

  linked_list_node_t * new_node = linked_list_create_node(value);
  while (!atomic_compare_exchange_strong(&(list->tail), &(tail), new_node)) {
    // do nothing
  }
  tail->next = new_node;
}


void linked_list_node_free(linked_list_node_t * node) {
  if (node->next != null) {
    linked_list_node_free(node->next);
    return;
  }

  free(node);
}

void linked_list_free(linked_list_t * list) {
  linked_list_node_free(list->head);
  free(list);
}

void linked_list_print(linked_list_t * list) {
  linked_list_node_t * current_node = atomic_load(&(list->head))->next;
  while (current_node != null) {
    printf("%lu ", current_node->value);
    current_node = current_node->next;
  }
}

struct thread_arg {
    size_t i;
    size_t k;
    linked_list_t * list;
};

void * thread_func(void * arg) {
  struct thread_arg * thread_arg = (struct thread_arg*)arg;
  for (size_t i = thread_arg->i * thread_arg->k;
      i < (thread_arg->i+1) * thread_arg->k;
      ++i) {
    linked_list_push(thread_arg->list, i);
    sched_yield();
  }
  return null;
}

int main(int argc, char * argv[]) {
  size_t N = strtol(argv[1], NULL, 10);
  size_t k = strtol(argv[2], NULL, 10);


  pthread_t * threads = malloc(sizeof(pthread_t) * N);
  struct thread_arg * thread_args = malloc(sizeof(struct thread_arg) * N);

  linked_list_t * list = linked_list_init();

  for (size_t i = 0; i < N ; ++i) {
    thread_args[i].i = i;
    thread_args[i].list = list;
    thread_args[i].k = k;
    pthread_create(&threads[i], null, thread_func, (void * )&thread_args[i]);
  }

  for (size_t i = 0; i < N; ++i) {
    pthread_join(threads[i], null);
  }

  linked_list_print(list);
  linked_list_free(list);

  free(threads); free(thread_args);
  return 0;
}