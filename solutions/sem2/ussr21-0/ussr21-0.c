#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>


typedef double (*function_t)(double );

int main(int argc, char * argv[]) {


  char * name = argv[1];
  char* func = argv[2];

  void * lib = dlopen(name, RTLD_NOW) ;

  if (!lib) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    exit(1);
  }

  void * func_ptr = dlsym(lib, func);

  if (!func_ptr) {
    fprintf(stderr, "dlsym: %s\n", dlerror());
  }

  function_t f = func_ptr;

  double x,y;

  while (scanf("%lf", &x) >=1) {
    y = f(x);
    printf("%.3f\n",y);
  }

  dlclose(lib);
}
