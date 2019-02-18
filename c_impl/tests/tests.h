#ifndef __TVM_TESTS_INCLUDED__
#define __TVM_TESTS_INCLUDED__

#include <stdio.h>

#define TEST_CASE(test_name, test_body)                                        \
  void test_name() {                                                           \
    test_body;                                                                 \
    printf("[Test - OK] " #test_name "\n");                                    \
  }

void value_test();

#endif