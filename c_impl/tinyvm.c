#include "tinyvm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  Vector *code = readFromFile("hw.tvm.compiled");

  VM *vm = new_VM();
  vm_execute(vm, code);

  return 0;
}
