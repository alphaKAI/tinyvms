#include "tinyvm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TValue *new_TValue() {
  TValue *tv = xmalloc(sizeof(TValue));
  tv->tt = Null;
  return tv;
}

TValue *new_TValue_with_tt(int tt) {
  TValue *tv = new_TValue();
  tv->tt = tt;
  return tv;
}

TValue *new_TValue_with_integer(long long int value) {
  TValue *tv = new_TValue_with_tt(Long);
  tv->value.integer = value;
  return tv;
}

TValue *new_TValue_with_str(StringBuilder *sb) {
  TValue *tv = new_TValue_with_tt(String);
  tv->value.str = sb;
  return tv;
}

TValue *new_TValue_with_bool(bool value) {
  TValue *tv = new_TValue_with_tt(Bool);
  tv->value.boolean = value;
  return tv;
}

TValue *new_TValue_with_array(TValueArray *array) {
  TValue *tv = new_TValue_with_tt(Array);
  tv->value.array = array;
  return tv;
}

TValue *new_TValue_with_func(VMFunction *func) {
  TValue *tv = new_TValue_with_tt(Function);
  tv->value.func = func;
  return tv;
}

long long int tv_getLong(TValue *tv) {
  assert(tv->tt == Long);
  return tv->value.integer;
}

StringBuilder *tv_getString(TValue *tv) {
  assert(tv->tt == String);
  return tv->value.str;
}

bool tv_getBool(TValue *tv) {
  assert(tv->tt == Bool);
  return tv->value.boolean;
}

TValueArray *getArray(TValue *tv) {
  assert(tv->tt == Array);
  return tv->value.array;
}

VMFunction *tv_getFunction(TValue *tv) {
  assert(tv->tt == Function);
  return tv->value.func;
}

bool tv_equals(TValue *this, TValue *that) {
  if (this->tt != that->tt) {
    fprintf(stderr, "Can not compare between incompatibility vtype \n");
    exit(EXIT_FAILURE);
  }

  switch (this->tt) {
  case Long:
    return this->value.integer == this->value.integer;
  case String:
    return !strcmp(sb_get(this->value.str), sb_get(that->value.str));
  case Bool:
    return this->value.boolean == that->value.boolean;
  case Array: {
    TValueArray *this_array = this->value.array;
    TValueArray *that_array = that->value.array;

    if (this_array->vec->len != that_array->vec->len) {
      return false;
    }

    for (int i = 0; i < this_array->vec->len; i++) {
      if (!tv_equals(this_array->vec->data[i], that_array->vec->data[i])) {
        return false;
      }
    }

    return true;
  }
  case Function:
    fprintf(stderr, "Con't compare with Function\n");
    exit(EXIT_FAILURE);
  case Null:
    fprintf(stderr, "Can't compare with Null");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Shouldn't reach here\n");
  exit(EXIT_FAILURE);
}

int vs_cmp(TValue *this, TValue *that) {
  if (this->tt != that->tt) {
    fprintf(stderr, "Can not compare between incompatibility vtype \n");
    exit(EXIT_FAILURE);
  }

  switch (this->tt) {
  case Long: {
    long long int c = this->value.integer, d = that->value.integer;

    if (c == d) {
      return 0;
    }
    if (c < d) {
      return -1;
    }
    return 1;
  }
  case String:
    return strcmp(sb_get(this->value.str), sb_get(that->value.str));
  case Bool:
    fprintf(stderr, "Can't compare with Bool\n");
    exit(EXIT_FAILURE);
  case Array:
    fprintf(stderr, "Can't compare with Array\n");
    exit(EXIT_FAILURE);
  case Function:
    fprintf(stderr, "Can't compare with Function\n");
    exit(EXIT_FAILURE);
  case Null:
    fprintf(stderr, "Can't compare with Null\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Shouldn't reach here\n");
  exit(EXIT_FAILURE);
}

TValue *tv_dup(TValue *tv) {
  switch (tv->tt) {
  case Long:
    return new_TValue_with_integer(tv->value.integer);
  case String:
    return new_TValue_with_str(sb_dup(tv->value.str));
  case Bool:
    return new_TValue_with_bool(tv->value.boolean);
  case Array:
    return new_TValue_with_array(tva_dup(tv->value.array));
  case Function:
    return new_TValue_with_func(vmf_dup(tv->value.func));
  default:
    return new_TValue();
  }
}

TValueArray *new_TValueArray() {
  TValueArray *array = xmalloc(sizeof(TValueArray));
  array->vec = new_vec();
  return array;
}

void tva_push(TValueArray *array, TValue *elem) { vec_push(array->vec, elem); }

TValue *tva_get(TValueArray *array, int idx) {
  assert(idx < array->vec->len);
  return array->vec->data[idx];
}

TValueArray *tva_dup(TValueArray *array) {
  TValueArray *ret = new_TValueArray();

  for (int i = 0; i < array->vec->len; i++) {
    tva_push(ret, tv_dup(array->vec->data[i]));
  }

  return ret;
}

VMFunction *new_VMFunction(StringBuilder *func_name, Vector *func_body,
                           Env *env) {
  VMFunction *func = xmalloc(sizeof(VMFunction));
  func->func_name = func_name;
  func->func_body = func_body;
  func->env = env;
  return func;
}

VMFunction *vmf_dup(VMFunction *func) {
  return new_VMFunction(func->func_name, func->func_body, env_dup(func->env));
}