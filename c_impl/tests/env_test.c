#include "tests.h"
#include "tinyvm.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>

TEST_CASE(integer_test, {
  Env *env = new_env();
  env_set(env, sdsnew("a"), new_TValue_with_integer(123));
  assert(tv_equals(env_get(env, sdsnew("a")), new_TValue_with_integer(123)));
})

TEST_CASE(str_test, {
  Env *env = new_env();
  env_set(env, sdsnew("a"), new_TValue_with_str(sdsnew("abc")));
  assert(
      tv_equals(env_get(env, sdsnew("a")), new_TValue_with_str(sdsnew("abc"))));
})

TEST_CASE(bool_test, {
  Env *env = new_env();
  env_set(env, sdsnew("a"), new_TValue_with_bool(true));
  assert(tv_equals(env_get(env, sdsnew("a")), new_TValue_with_bool(true)));
})

TEST_CASE(array_test, {
  Env *env = new_env();

  Vector *a = new_vec();
  Vector *b = new_vec();

  for (int i = 0; i < 10; i++) {
    vec_push(a, new_TValue_with_integer(i));
    vec_push(b, new_TValue_with_integer(i));
  }

  env_set(env, sdsnew("arr"), new_TValue_with_array(a));
  assert(tv_equals(env_get(env, sdsnew("arr")), new_TValue_with_array(b)));
})

TEST_CASE(recursive_test, {
  Env *base_env = new_env();

  env_set(base_env, sdsnew("a"), new_TValue_with_integer(123));
  assert(env_has(base_env, sdsnew("a")) == true);

  Env *derived_env = env_dup(base_env);
  assert(env_has(derived_env, sdsnew("a")) == true);
  env_set(derived_env, sdsnew("b"), new_TValue_with_integer(456));
  assert(env_has(derived_env, sdsnew("b")) == true);
  assert(env_has(base_env, sdsnew("b")) == false);
})

void env_test() {
  integer_test();
  str_test();
  bool_test();
  array_test();
  recursive_test();

  printf("[env_test] All of tests are passed\n");
}