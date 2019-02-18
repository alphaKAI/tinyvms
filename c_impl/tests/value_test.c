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
  Vector *array = new_vec();

  for (int i = 0; i < 10; i++) {
    vec_push(array, new_TValue_with_integer(i));
  }

  for (int i = 0; i < 10; i++) {
    assert(tv_getLong(array->data[i]) == i);
  }

  array = new_vec();
  size_t strs_len = 3;
  sds *strs = xmalloc(sizeof(sds) * strs_len);
  strs[0] = sdsnew("abc");
  strs[1] = sdsnew("def");
  strs[2] = sdsnew("ghi");
  for (size_t i = 0; i < strs_len; i++) {
    vec_push(array, new_TValue_with_str(strs[i]));
  }
  for (size_t i = 0; i < strs_len; i++) {
    assert(tv_equals(array->data[i], new_TValue_with_str(strs[i])));
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

void value_test() {
  test_null();
  test_integer();
  test_str();
  test_bool();
  test_array();
  test_eq();
  test_cmp();

  printf("[value_test] All of tests are passed\n");
}
