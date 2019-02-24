#include "tests.h"
#include "tinyvm.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>

TEST_CASE(test_null, {
  TValue *tv = new_TValue();
  assert(tv->tt == Null);
})

TEST_CASE(test_integer, {
  TValue *tv = new_TValue_with_integer(123);
  assert(tv_getLong(tv) == 123);
})

TEST_CASE(test_str, {
  TValue *tv = new_TValue_with_str("abcdef");
  assert(strcmp("abcdef", tv_getString(tv)) == 0);
})

TEST_CASE(test_bool, {
  TValue *tv = new_TValue_with_bool(false);
  assert(tv_getBool(tv) == false);
})

TEST_CASE(test_array, {
  Vector *vec = new_vec();

  for (int i = 0; i < 10; i++) {
    vec_push(vec, new_TValue_with_integer(i));
  }

  TValue *array = new_TValue_with_array(vec);
  Vector *tv_array = tv_getArray(array);

  for (int i = 0; i < 10; i++) {
    assert(tv_getLong(tv_array->data[i]) == i);
  }

  vec = new_vec();
  size_t strs_len = 3;
  sds *strs = xmalloc(sizeof(sds) * strs_len);
  strs[0] = sdsnew("abc");
  strs[1] = sdsnew("def");
  strs[2] = sdsnew("ghi");
  for (size_t i = 0; i < strs_len; i++) {
    vec_push(vec, new_TValue_with_str(strs[i]));
  }

  array = new_TValue_with_array(vec);
  tv_array = tv_getArray(array);

  for (size_t i = 0; i < strs_len; i++) {
    assert(tv_equals(tv_array->data[i], new_TValue_with_str(strs[i])));
  }
})

TEST_CASE(test_eq, {
  TValue *a = new_TValue_with_integer(1);
  TValue *b = new_TValue_with_integer(1);
  TValue *c = new_TValue_with_integer(2);
  assert(tv_equals(a, b) == true);
  assert(tv_equals(a, c) == false);

  a = new_TValue_with_str("abc");
  b = new_TValue_with_str("abc");
  c = new_TValue_with_str("def");
  assert(tv_equals(a, b) == true);
  assert(tv_equals(a, c) == false);

  a = new_TValue_with_bool(true);
  b = new_TValue_with_bool(true);
  c = new_TValue_with_bool(false);
  assert(tv_equals(a, b) == true);
  assert(tv_equals(a, c) == false);

  Vector *va = new_vec();
  Vector *vb = new_vec();
  Vector *vc = new_vec();
  TValue *t;
  TValue *t2;
  for (int i = 0; i < 10; i++) {
    t = new_TValue_with_integer(i);
    t2 = new_TValue_with_integer(9 - i);
    vec_push(va, t);
    vec_push(vb, t);
    vec_push(vc, t2);
  }

  TValue *ta = new_TValue_with_array(va);
  TValue *tb = new_TValue_with_array(vb);
  TValue *tc = new_TValue_with_array(vc);

  assert(tv_equals(ta, tb) == true);
  assert(tv_equals(ta, tc) == false);
})

TEST_CASE(test_cmp, {
  TValue *a = new_TValue_with_integer(1);
  TValue *b = new_TValue_with_integer(1);
  TValue *c = new_TValue_with_integer(2);

  assert(tv_cmp(a, b) == 0);
  assert(tv_cmp(a, c) == -1);
  assert(tv_cmp(c, a) == 1);

  a = new_TValue_with_str("abc");
  b = new_TValue_with_str("abc");
  c = new_TValue_with_str("def");

  assert(tv_cmp(a, b) == 0);
  assert(tv_cmp(a, c) == -1);
  assert(tv_cmp(c, a) == 1);
});

TEST_CASE(test_cmps, {
  TValue *a = new_TValue_with_integer(1);
  TValue *b = new_TValue_with_integer(1);
  TValue *c = new_TValue_with_integer(2);

  assert(tv_lt(a, b) == false);
  assert(tv_lt(a, c) == true);
  assert(tv_lt(c, a) == false);
  assert(tv_lte(a, b) == true);
  assert(tv_lte(a, c) == true);
  assert(tv_lte(c, a) == false);
  assert(tv_gt(a, b) == false);
  assert(tv_gt(a, c) == false);
  assert(tv_gt(c, a) == true);
  assert(tv_gte(a, b) == true);
  assert(tv_gte(a, c) == false);
  assert(tv_gte(c, a) == true);

  a = new_TValue_with_str("abc");
  b = new_TValue_with_str("abc");
  c = new_TValue_with_str("def");

  assert(tv_lt(a, b) == false);
  assert(tv_lt(a, c) == true);
  assert(tv_lt(c, a) == false);
  assert(tv_lte(a, b) == true);
  assert(tv_lte(a, c) == true);
  assert(tv_lte(c, a) == false);
  assert(tv_gt(a, b) == false);
  assert(tv_gt(a, c) == false);
  assert(tv_gt(c, a) == true);
  assert(tv_gte(a, b) == true);
  assert(tv_gte(a, c) == false);
  assert(tv_gte(c, a) == true);
})

TEST_CASE(test_logics, {
  TValue *tv_true = new_TValue_with_bool(true);
  TValue *tv_false = new_TValue_with_bool(false);

  assert(tv_and(tv_true, tv_true) == true);
  assert(tv_and(tv_true, tv_false) == false);
  assert(tv_and(tv_false, tv_true) == false);
  assert(tv_and(tv_false, tv_false) == false);

  assert(tv_or(tv_true, tv_true) == true);
  assert(tv_or(tv_true, tv_false) == true);
  assert(tv_or(tv_false, tv_true) == true);
  assert(tv_or(tv_false, tv_false) == false);
})

void value_test() {
  test_null();
  test_integer();
  test_str();
  test_bool();
  test_array();
  test_eq();
  test_cmp();
  test_cmps();
  test_logics();

  printf("[value_test] All of tests are passed\n");
}
