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
})

void value_test() {
  test_null();
  test_integer();
  test_str();
  test_bool();
  test_array();

  printf("[value_test] All of tests are passed\n");
}

// TValue *new_TValue_with_func(VMFunction *func) {
