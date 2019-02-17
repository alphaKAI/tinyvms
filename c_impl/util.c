#include "tinyvm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vector *new_vec() {
  Vector *v = malloc(sizeof(Vector));
  v->data = malloc(sizeof(void *) * 16);
  v->capacity = 16;
  v->len = 0;
  return v;
}

void vec_free(Vector **v_ptr) {
  free((*v_ptr)->data);
  free(*v_ptr);
  *v_ptr = NULL;
}

void vec_expand(Vector *v, long long int size) {
  if (v->len < size) {
    v->capacity = size;
    v->data = realloc(v->data, sizeof(void *) * v->capacity);
  }
}

void vec_push(Vector *v, void *elem) {
  if (v->len == v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, sizeof(void *) * v->capacity);
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

bool vec_containss(Vector *v, char *key) {
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

void *vec_get(Vector *v, int idx) {
  assert(idx < v->len);
  return v->data[idx];
}

Map *new_map(void) {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vec();
  map->vals = new_vec();
  return map;
}

void map_free(Map **map_ptr) {
  vec_free(&(*map_ptr)->keys);
  vec_free(&(*map_ptr)->vals);
  free(*map_ptr);
  *map_ptr = NULL;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void map_puti(Map *map, char *key, int val) {
  map_put(map, key, (void *)(intptr_t)val);
}

void *map_get(Map *map, char *key) {
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (!strcmp(map->keys->data[i], key))
      return map->vals->data[i];
  return NULL;
}

int map_geti(Map *map, char *key, int default_) {
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (!strcmp(map->keys->data[i], key))
      return (intptr_t)map->vals->data[i];
  return default_;
}

StringBuilder *new_sb(void) {
  StringBuilder *sb = malloc(sizeof(StringBuilder));
  sb->data = malloc(8);
  sb->capacity = 8;
  sb->len = 0;
  return sb;
}

StringBuilder *new_sb_with_char(char *buf) {
  StringBuilder *sb = new_sb();
  sb_append(sb, buf);
  return sb;
}

static void sb_grow(StringBuilder *sb, int len) {
  if (sb->len + len <= sb->capacity)
    return;

  while (sb->len + len > sb->capacity)
    sb->capacity *= 2;
  sb->data = realloc(sb->data, sb->capacity);
}

void sb_add(StringBuilder *sb, char c) {
  sb_grow(sb, 1);
  sb->data[sb->len++] = c;
}

void sb_append(StringBuilder *sb, char *s) { sb_append_n(sb, s, strlen(s)); }

void sb_append_n(StringBuilder *sb, char *s, int len) {
  sb_grow(sb, len);
  memcpy(sb->data + sb->len, s, len);
  sb->len += len;
}

char *sb_get(StringBuilder *sb) {
  sb_add(sb, '\0');
  return sb->data;
}

StringBuilder *sb_dup(StringBuilder *sb) {
  StringBuilder *ret = new_sb();
  sb_append(ret, sb->data);
  return ret;
}

void *xmalloc(size_t size) {
  void *ptr = malloc(size);

  if (ptr == NULL) {
    fprintf(stderr, "Failed to allocate memory <size:%ld>\n", size);
    exit(EXIT_FAILURE);
  }

  return ptr;
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