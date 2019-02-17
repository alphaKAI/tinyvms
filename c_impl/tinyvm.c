#include "tinyvm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "too few arguments\n");
    exit(EXIT_FAILURE);
  }

  Vector *code = readFromFile(argv[1]);

  printf("code : \n");
  code_printer(code);

  VM *vm = new_VM();
  vm_execute(vm, code);

  return 0;
}
