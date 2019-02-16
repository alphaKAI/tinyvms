#include "tinyvm.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct ST {
  struct ST *super;
  int i;
} ST;

ST *new_ST(int i) {
  ST *st = xmalloc(sizeof(ST));

  st->super = NULL;
  st->i = i;

  return st;
}

ST *new_ST_with_super(int i, ST *super) {
  ST *st = new_ST(i);
  st->super = super;
  return st;
}

void print_st(ST *st) { printf("ST<i : %d>\n", st->i); }

#include <stdlib.h>

int main(int argc, char const *argv[]) {
  ST *st = new_ST(0);
  ST *tmp = st;
  for (int i = 1; i < 10; i++) {
    tmp = new_ST_with_super(i, tmp);
  }

  for (ST *s = tmp; s != NULL; s = s->super) {
    print_st(s);
  }

  Vector *vec = new_vec();
  for (int i = 0; i < 10; i++) {
    vec_pushi(vec, i);
  }
  for (int i = 0; i < 10; i++) {
    printf("vec[%d] : %d\n", i, (int)vec->data[i]);
  }

  return 0;
}
