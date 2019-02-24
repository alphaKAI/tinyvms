#include "tinyvm.h"
#include <assert.h>

#ifdef __USE_BOEHM_GC__
#include <gc.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vector *new_vec() {
  Vector *v = xmalloc(sizeof(Vector));
  v->data = xmalloc(sizeof(void *) * 16);
  v->capacity = 16;
  v->len = 0;
  return v;
}

void vec_expand(Vector *v, long long int size) {
  if (v->len < size) {
    v->capacity = size;
    v->len = size;
#ifdef __USE_BOEHM_GC__
    v->data = GC_REALLOC(v->data, sizeof(void *) * v->capacity);
#else
    v->data = realloc(v->data, sizeof(void *) * v->capacity);
#endif
  }
}

void vec_push(Vector *v, void *elem) {
  if (v->len == v->capacity) {
    v->capacity *= 2;
#ifdef __USE_BOEHM_GC__
    v->data = GC_REALLOC(v->data, sizeof(void *) * v->capacity);
#else
    v->data = realloc(v->data, sizeof(void *) * v->capacity);
#endif
  }
  v->data[v->len++] = elem;
}

void vec_pushi(Vector *v, int val) { vec_push(v, (void *)(intptr_t)val); }

void *vec_pop(Vector *v) {
  assert(v->len);
  return v->data[--v->len];
}

void *vec_last(Vector *v) {
  assert(v->len);
  return v->data[v->len - 1];
}

bool vec_contains(Vector *v, void *elem) {
  for (int i = 0; i < v->len; i++)
    if (v->data[i] == elem)
      return true;
  return false;
}

bool vec_containss(Vector *v, sds key) {
  for (int i = 0; i < v->len; i++)
    if (!strcmp(v->data[i], key))
      return true;
  return false;
}

bool vec_union1(Vector *v, void *elem) {
  if (vec_contains(v, elem))
    return false;
  vec_push(v, elem);
  return true;
}

void *vec_get(Vector *v, long long int idx) {
  assert(idx < v->len);
  return v->data[idx];
}

Vector *vec_dup(Vector *v) {
  Vector *vec = new_vec();
  for (int i = 0; i < v->len; i++) {
    vec_push(vec, v->data[i]);
  }
  return vec;
}

Map *new_map(void) {
  Map *map = xmalloc(sizeof(Map));
  map->tree = new_AVLTree();
  return map;
}

static int map_compare(sds a, sds b) {
  int ret = strcmp(a, b);
  if (ret < 0) {
    return -1;
  } else if (ret == 0) {
    return 0;
  } else {
    return 1;
  }
}

void map_put(Map *map, sds key, void *val) {
  avl_insert(map->tree, key, val, (ELEM_COMPARE)&map_compare);
}

void map_puti(Map *map, sds key, int val) {
  map_put(map, key, (void *)(intptr_t)val);
}

void *map_get(Map *map, sds key) {
  return avl_find(map->tree, key, (ELEM_COMPARE)&map_compare);
}

inline void *xmalloc(size_t size) {
#ifdef __USE_BOEHM_GC__
  void *ptr = GC_MALLOC(size);
#else
  void *ptr = malloc(size);
#endif

  if (ptr == NULL) {
    fprintf(stderr, "Failed to allocate memory <size:%ld>\n", size);
    exit(EXIT_FAILURE);
  }

  return ptr;
}

inline void xfree(void *ptr) {
  if (ptr != NULL) {
#ifdef __USE_BOEHM_GC__
    GC_FREE(ptr);
#else
    free(ptr);
#endif
  } else {
    fprintf(stderr, "xfree got NULL pointer\n");
    exit(EXIT_FAILURE);
  }
}

#define case_printer(case_name)                                                \
  case case_name:                                                              \
    printf(#case_name);                                                        \
    break

void type_print(int type) {
  switch (type) {
    case_printer(tOpVariableDeclareOnlySymbol);
    case_printer(tOpVariableDeclareWithAssign);
    case_printer(tOpPop);
    case_printer(tOpPush);
    case_printer(tOpAdd);
    case_printer(tOpSub);
    case_printer(tOpMul);
    case_printer(tOpDiv);
    case_printer(tOpMod);
    case_printer(tOpReturn);
    case_printer(tOpGetVariable);
    case_printer(tOpSetVariablePop);
    case_printer(tOpSetArrayElement);
    case_printer(tOpGetArrayElement);
    case_printer(tOpMakeArray);
    case_printer(tOpCall);
    case_printer(tOpNop);
    case_printer(tOpFunctionDeclare);
    case_printer(tOpEqualExpression);
    case_printer(tOpNotEqualExpression);
    case_printer(tOpLtExpression);
    case_printer(tOpLteExpression);
    case_printer(tOpGtExpression);
    case_printer(tOpGteExpression);
    case_printer(tOpAndExpression);
    case_printer(tOpOrExpression);
    case_printer(tOpXorExpression);
    case_printer(tOpJumpRel);
    case_printer(tOpJumpAbs);
    case_printer(tOpPrint);
    case_printer(tOpPrintln);
    case_printer(tOpIFStatement);
    case_printer(tOpAssignExpression);
    case_printer(tOpAssert);
    case_printer(tIValue);
  }
}
