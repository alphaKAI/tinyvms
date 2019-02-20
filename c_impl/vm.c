#include "sds/sds.h"
#include "tinyvm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

VM *new_VM() {
  VM *vm = xmalloc(sizeof(VM));
  vm->env = new_env();
  vm->stack = new_vec();

  /* builtin funcs */

  TValue *func_tv;
  Vector *func_body;

  /* print */
  func_body = new_vec();
  vec_pushi(func_body, tOpPrint);

  func_tv =
      new_TValue_with_func(new_VMFunction(sdsnew("print"), func_body, vm->env));
  env_def(vm->env, sdsnew("print"), func_tv);

  /* println */
  func_body = new_vec();
  vec_pushi(func_body, tOpPrintln);

  func_tv = new_TValue_with_func(
      new_VMFunction(sdsnew("println"), func_body, vm->env));
  env_def(vm->env, sdsnew("println"), func_tv);

  return vm;
}

TValue *vm_stackPeekTop(VM *vm) {
  if (vm->stack->len > 0) {
    return (TValue *)vec_last(vm->stack);
  } else {
    return NULL;
  }
}

#define VM_ERROR(msg)                                                          \
  {                                                                            \
    fprintf(stderr, "<VM-ERROR> %s\n", msg);                                   \
    exit(EXIT_FAILURE);                                                        \
  }

#define VM_ASSERT(expr, msg)                                                   \
  {                                                                            \
    if (!(expr)) {                                                             \
      VM_ERROR(msg);                                                           \
    }                                                                          \
  }

#define VM_ASSERT0(expr)                                                       \
  {                                                                            \
    if (!(expr)) {                                                             \
      VM_ERROR("");                                                            \
    }                                                                          \
  }

#define __TINYVM_DEBUG__

#ifdef __TINYVM_DEBUG__
#define VM_DEBUG_PRINT(vm, op)                                                 \
  {                                                                            \
    printf("[DEBUG]-----------------------------------------------\n");        \
    printf("op: ");                                                            \
    type_print(op);                                                            \
    printf("\n");                                                              \
    Vector *keys = vm->env->vs->store->keys;                                   \
    printf("keys->len : %lld\n", keys->len);                                   \
    for (int i = 0; i < keys->len; i++) {                                      \
      printf("key : %s, ", keys->data[i]);                                     \
      printf("val : ");                                                        \
      tv_print(env_get(vm->env, keys->data[i]));                               \
      printf("\n");                                                            \
    }                                                                          \
    printf("vm->stack->data : %p\n", vm->stack->data);                         \
    printf("stack : [");                                                       \
    for (int i = 0; i < vm->stack->len; i++) {                                 \
      if (i > 0) {                                                             \
        printf(", ");                                                          \
      }                                                                        \
      tv_print(vm->stack->data[i]);                                            \
    }                                                                          \
    printf("]\n");                                                             \
  }
#else
#define VM_DEBUG_PRINT(vm, op)
#endif

#define __ENABLE_DIRECT_THREADED_CODE__

#ifdef __ENABLE_DIRECT_THREADED_CODE__
TValue *vm_execute(VM *vm, Vector *code) {
  size_t pc = 0;
  long long int op = 0;

  static void *table[] = {&&L_tOpVariableDeclareOnlySymbol,
                          &&L_tOpVariableDeclareWithAssign,
                          &&L_tOpPop,
                          &&L_tOpPush,
                          &&L_tOpAdd,
                          &&L_tOpSub,
                          &&L_tOpMul,
                          &&L_tOpDiv,
                          &&L_tOpMod,
                          &&L_tOpReturn,
                          &&L_tOpGetVariable,
                          &&L_tOpSetVariablePop,
                          &&L_tOpSetArrayElement,
                          &&L_tOpGetArrayElement,
                          &&L_tOpMakeArray,
                          &&L_tOpCall,
                          &&L_tOpNop,
                          &&L_tOpFunctionDeclare,
                          &&L_tOpEqualExpression,
                          &&L_tOpNotEqualExpression,
                          &&L_tOpLtExpression,
                          &&L_tOpLteExpression,
                          &&L_tOpGtExpression,
                          &&L_tOpGteExpression,
                          &&L_tOpAndExpression,
                          &&L_tOpOrExpression,
                          &&L_tOpXorExpression,
                          &&L_tOpJumpRel,
                          &&L_tOpJumpAbs,
                          &&L_tOpPrint,
                          &&L_tOpPrintln,
                          &&L_tOpIFStatement,
                          &&L_tOpAssignExpression,
                          &&L_tOpAssert,
                          &&L_tIValue};

  long long int table_len = sizeof(table) / sizeof(table[0]);

  void **ops_ptr = xmalloc(sizeof(void *) * (code->len + 1));
  for (int j = 0; j < code->len; j++) {
    long long int idx = (long long int)code->data[j];
    if (idx < table_len) {
      ops_ptr[j] = table[idx];
    }
  }
  ops_ptr[code->len] = &&L_end;

#define DTHC_CASE(op_name, proc_code)                                          \
  L_##op_name : {                                                              \
    VM_DEBUG_PRINT(vm, op);                                                    \
    proc_code;                                                                 \
    pc++;                                                                      \
    goto *ops_ptr[pc];                                                         \
  }

  /* L_start */
  op = (long long int)code->data[0];
  goto *table[op];

  DTHC_CASE(tOpVariableDeclareOnlySymbol, {
    TValue *symbol = (TValue *)code->data[pc++ + 1];
    env_def(vm->env, tv_getString(symbol), new_TValue());
  })

  DTHC_CASE(tOpVariableDeclareWithAssign, {
    TValue *symbol = (TValue *)code->data[pc++ + 1];
    TValue *v = (TValue *)vec_pop(vm->stack);
    env_def(vm->env, tv_getString(symbol), v);
  })

  DTHC_CASE(tOpAssignExpression, {
    TValue *symbol = (TValue *)code->data[pc++ + 1];
    TValue *v = (TValue *)vec_pop(vm->stack);
    env_def(vm->env, tv_getString(symbol), v);
  })

  DTHC_CASE(tOpPush, {
    TValue *v = (TValue *)code->data[pc++ + 1];
    VM_ASSERT(v != NULL, "Execute Error on tOpPush");
    vec_push(vm->stack, v);
  })

  DTHC_CASE(tOpPop, { vec_pop(vm->stack); })

  DTHC_CASE(tOpAdd, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    VM_ASSERT0(a->tt == b->tt && a->tt == Long);
    vec_push(vm->stack,
             new_TValue_with_integer(a->value.integer + b->value.integer));
  })

  DTHC_CASE(tOpSub, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    assert(a->tt == b->tt && a->tt == Long);
    vec_push(vm->stack,
             new_TValue_with_integer(a->value.integer - b->value.integer));
  })

  DTHC_CASE(tOpMul, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    assert(a->tt == b->tt && a->tt == Long);
    vec_push(vm->stack,
             new_TValue_with_integer(a->value.integer * b->value.integer));
  })

  DTHC_CASE(tOpDiv, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    assert(a->tt == b->tt && a->tt == Long);
    vec_push(vm->stack,
             new_TValue_with_integer(a->value.integer / b->value.integer));
  })

  DTHC_CASE(tOpMod, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    assert(a->tt == b->tt && a->tt == Long);
    vec_push(vm->stack,
             new_TValue_with_integer(a->value.integer % b->value.integer));
  })

  DTHC_CASE(tOpReturn, { return vm_stackPeekTop(vm); })

  DTHC_CASE(tOpGetVariable, {
    TValue *v = (TValue *)code->data[pc++ + 1];
    VM_ASSERT(v != NULL, "Execute Error on tOpGetVariable");
    HasPtrResult *ptr = env_has_ptr(vm->env, tv_getString(v));
    if (ptr->tv != NULL) {
      vec_push(vm->stack, ptr->tv);
    } else {
      fprintf(stderr, "No such a variable %s", tv_getString(v));
      exit(EXIT_FAILURE);
    }
  })

  DTHC_CASE(tOpSetVariablePop, {
    TValue *dst = (TValue *)code->data[pc++ + 1];
    TValue *v = (TValue *)vec_pop(vm->stack);
    env_set(vm->env, tv_getString(dst), v);
  })

  DTHC_CASE(tOpCall, {
    TValue *func = (TValue *)code->data[pc++ + 1];
    sds fname = tv_getString(func);
    Env *cpyEnv = vm->env;
    vm->env = env_dup(tv_getFunction(env_get(vm->env, fname))->env);
    vm_execute(vm, tv_getFunction(env_get(cpyEnv, fname))->func_body);
    vm->env = cpyEnv;
  })

  DTHC_CASE(tOpNop, {})

  DTHC_CASE(tOpFunctionDeclare, {
    TValue *symbol = (TValue *)code->data[pc++ + 1];
    sds func_name = tv_getString(symbol);
    TValue *op_blocks_length = (TValue *)code->data[pc++ + 1];
    Vector *func_body = new_vec();
    for (int i = 0; i < tv_getLong(op_blocks_length); i++) {
      vec_push(func_body, code->data[pc++ + 1]);
    }
    env_def(vm->env, func_name,
            new_TValue_with_func(
                new_VMFunction(func_name, func_body, env_dup(vm->env))));
  })

  DTHC_CASE(tOpEqualExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_equals(a, b)));
  })

  DTHC_CASE(tOpNotEqualExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(!tv_equals(a, b)));
  })

  DTHC_CASE(tOpLtExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_lt(a, b)));
  })
  DTHC_CASE(tOpLteExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_lte(a, b)));
  })

  DTHC_CASE(tOpGtExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_gt(a, b)));
  })

  DTHC_CASE(tOpGteExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_gte(a, b)));
  })

  DTHC_CASE(tOpAndExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_and(a, b)));
  })

  DTHC_CASE(tOpOrExpression, {
    TValue *a = (TValue *)vec_pop(vm->stack);
    TValue *b = (TValue *)vec_pop(vm->stack);
    vec_push(vm->stack, new_TValue_with_bool(tv_or(a, b)));
  })

  DTHC_CASE(tOpXorExpression, { VM_ERROR("Not implemented <XOR>"); })

  DTHC_CASE(tOpPrint, {
    TValue *v = (TValue *)vec_pop(vm->stack);
    tv_print(v);
  })

  DTHC_CASE(tOpPrintln, {
    TValue *v = (TValue *)vec_pop(vm->stack);
    tv_print(v);
    printf("\n");
  })

  DTHC_CASE(tOpJumpRel, {
    TValue *v = (TValue *)code->data[pc++ + 1];
    pc += tv_getLong(v);
  })

  DTHC_CASE(tOpJumpAbs, {
    TValue *v = (TValue *)code->data[pc++ + 1];
    pc = tv_getLong(v);
  })

  DTHC_CASE(tOpIFStatement, {
    TValue *cond = (TValue *)vec_pop(vm->stack);
    bool condResult;
    switch (cond->tt) {
    case Long:
      condResult = tv_getLong(cond) != 0;
      break;
    case Bool:
      condResult = tv_getBool(cond);
      break;
    case String:
      VM_ERROR("Execute Error Invalid Condition <string>");
    case Array:
      VM_ERROR("Execute Error Invalid Condition <array>");
    case Function:
      VM_ERROR("Execute Error Invalid Condition <function>");
    case Null:
      condResult = false;
      break;
    }

    long long int trueBlockLength = tv_getLong((TValue *)code->data[pc++ + 1]);

    if (!condResult) {
      pc += trueBlockLength;
    }
  })

  DTHC_CASE(tOpSetArrayElement, {
    sds variable = tv_getString((TValue *)code->data[pc++ + 1]);
    long long int idx = tv_getLong((TValue *)vec_pop(vm->stack));
    TValue *val = (TValue *)vec_pop(vm->stack);
    Vector *array = tv_getArray(env_get(vm->env, variable));
    array->data[idx] = val;
  })

  DTHC_CASE(tOpGetArrayElement, {
    sds variable = tv_getString((TValue *)code->data[pc++ + 1]);
    long long int idx = tv_getLong((TValue *)vec_pop(vm->stack));
    vec_push(vm->stack, vec_get(tv_getArray(env_get(vm->env, variable)), idx));
  })

  DTHC_CASE(tOpMakeArray, {
    long long int array_size = tv_getLong((TValue *)code->data[pc++ + 1]);
    Vector *array = new_vec();
    vec_expand(array, array_size);
    for (int i = array_size - 1; i >= 0; i--) {
      array->data[i] = (TValue *)vec_pop(vm->stack);
    }
    vec_push(vm->stack, new_TValue_with_array(array));
  })

  DTHC_CASE(tIValue, { VM_ERROR("TValue* should not peek directly"); })

  DTHC_CASE(tOpAssert, {
    sds msg = tv_getString((TValue *)vec_pop(vm->stack));
    bool result = tv_getBool((TValue *)vec_pop(vm->stack));
    if (!result) {
      VM_ERROR(msg);
    }
  })

L_end:
  return vm_stackPeekTop(vm);
}
#else
TValue *vm_execute(VM *vm, Vector *code) {
  for (long long int pc = 0; pc < code->len; pc++) {
    int op = (int)code->data[pc];

    VM_DEBUG_PRINT(vm, op);

    switch (op) {
    case tOpVariableDeclareOnlySymbol: {
      TValue *symbol = (TValue *)code->data[pc++ + 1];
      env_def(vm->env, tv_getString(symbol), new_TValue());
      break;
    }
    case tOpVariableDeclareWithAssign: {
      TValue *symbol = (TValue *)code->data[pc++ + 1];
      TValue *v = (TValue *)vec_pop(vm->stack);
      env_def(vm->env, tv_getString(symbol), v);
      break;
    }
    case tOpAssignExpression: {
      TValue *symbol = (TValue *)code->data[pc++ + 1];
      TValue *v = (TValue *)vec_pop(vm->stack);
      env_def(vm->env, tv_getString(symbol), v);
      break;
    }
    case tOpPush: {
      TValue *v = (TValue *)code->data[pc++ + 1];
      VM_ASSERT(v != NULL, "Execute Error on tOpPush");
      vec_push(vm->stack, v);
      break;
    }
    case tOpPop: {
      vec_pop(vm->stack);
      break;
    }
    case tOpAdd: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      VM_ASSERT0(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack,
               new_TValue_with_integer(a->value.integer + b->value.integer));
      break;
    }
    case tOpSub: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack,
               new_TValue_with_integer(a->value.integer - b->value.integer));
      break;
    }
    case tOpMul: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack,
               new_TValue_with_integer(a->value.integer * b->value.integer));
      break;
    }
    case tOpDiv: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack,
               new_TValue_with_integer(a->value.integer / b->value.integer));
      break;
    }
    case tOpMod: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack,
               new_TValue_with_integer(a->value.integer % b->value.integer));
      break;
    }
    case tOpReturn: {
      return vm_stackPeekTop(vm);
    }
    case tOpGetVariable: {
      TValue *v = (TValue *)code->data[pc++ + 1];
      VM_ASSERT(v != NULL, "Execute Error on tOpGetVariable");
      HasPtrResult *ptr = env_has_ptr(vm->env, tv_getString(v));
      if (ptr->tv != NULL) {
        vec_push(vm->stack, ptr->tv);
      } else {
        fprintf(stderr, "No such a variable %s", tv_getString(v));
        exit(EXIT_FAILURE);
      }
      break;
    }
    case tOpSetVariablePop: {
      TValue *dst = (TValue *)code->data[pc++ + 1];
      TValue *v = (TValue *)vec_pop(vm->stack);
      env_set(vm->env, tv_getString(dst), v);
      break;
    }
    case tOpCall: {
      TValue *func = (TValue *)code->data[pc++ + 1];
      sds fname = tv_getString(func);
      Env *cpyEnv = vm->env;
      vm->env = env_dup(tv_getFunction(env_get(vm->env, fname))->env);
      vm_execute(vm, tv_getFunction(env_get(cpyEnv, fname))->func_body);
      vm->env = cpyEnv;
      break;
    }
    case tOpNop: {
      break;
    }
    case tOpFunctionDeclare: {
      TValue *symbol = (TValue *)code->data[pc++ + 1];
      sds func_name = tv_getString(symbol);
      TValue *op_blocks_length = (TValue *)code->data[pc++ + 1];
      Vector *func_body = new_vec();
      for (int i = 0; i < tv_getLong(op_blocks_length); i++) {
        vec_push(func_body, code->data[pc++ + 1]);
      }
      env_def(vm->env, func_name,
              new_TValue_with_func(
                  new_VMFunction(func_name, func_body, env_dup(vm->env))));
      break;
    }
    case tOpEqualExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_equals(a, b)));
      break;
    }
    case tOpNotEqualExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(!tv_equals(a, b)));
      break;
    }
    case tOpLtExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_lt(a, b)));
      break;
    }
    case tOpLteExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_lte(a, b)));
      break;
    }
    case tOpGtExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_gt(a, b)));
      break;
    }
    case tOpGteExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_gte(a, b)));
      break;
    }
    case tOpAndExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_and(a, b)));
      break;
    }
    case tOpOrExpression: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new_TValue_with_bool(tv_or(a, b)));
      break;
    }
    case tOpXorExpression: {
      VM_ERROR("Not implemented <XOR>");
    }
    case tOpPrint: {
      TValue *v = (TValue *)vec_pop(vm->stack);
      tv_print(v);
      break;
    }
    case tOpPrintln: {
      TValue *v = (TValue *)vec_pop(vm->stack);
      tv_print(v);
      printf("\n");
      break;
    }
    case tOpJumpRel: {
      TValue *v = (TValue *)code->data[pc++ + 1];
      pc += tv_getLong(v);
      break;
    }
    case tOpJumpAbs: {
      TValue *v = (TValue *)code->data[pc++ + 1];
      pc = tv_getLong(v);
      break;
    }
    case tOpIFStatement: {
      TValue *cond = (TValue *)vec_pop(vm->stack);
      bool condResult;
      switch (cond->tt) {
      case Long:
        condResult = tv_getLong(cond) != 0;
        break;
      case Bool:
        condResult = tv_getBool(cond);
        break;
      case String:
        VM_ERROR("Execute Error Invalid Condition <string>");
      case Array:
        VM_ERROR("Execute Error Invalid Condition <array>");
      case Function:
        VM_ERROR("Execute Error Invalid Condition <function>");
      case Null:
        condResult = false;
        break;
      }

      long long int trueBlockLength =
          tv_getLong((TValue *)code->data[pc++ + 1]);

      if (condResult) {
        break;
      } else {
        pc += trueBlockLength;
      }
      break;
    }
    case tOpSetArrayElement: {
      sds variable = tv_getString((TValue *)code->data[pc++ + 1]);
      long long int idx = tv_getLong((TValue *)vec_pop(vm->stack));
      TValue *val = (TValue *)vec_pop(vm->stack);
      Vector *array = tv_getArray(env_get(vm->env, variable));
      array->data[idx] = val;
      break;
    }
    case tOpGetArrayElement: {
      sds variable = tv_getString((TValue *)code->data[pc++ + 1]);
      long long int idx = tv_getLong((TValue *)vec_pop(vm->stack));
      vec_push(vm->stack,
               vec_get(tv_getArray(env_get(vm->env, variable)), idx));
      break;
    }
    case tOpMakeArray: {
      long long int array_size = tv_getLong((TValue *)code->data[pc++ + 1]);
      Vector *array = new_vec();
      vec_expand(array, array_size);
      for (int i = array_size - 1; i >= 0; i--) {
        array->data[i] = (TValue *)vec_pop(vm->stack);
      }
      vec_push(vm->stack, new_TValue_with_array(array));
      break;
    }
    case tIValue:
      VM_ERROR("TValue* should not peek directly");
    case tOpAssert: {
      sds msg = tv_getString((TValue *)vec_pop(vm->stack));
      bool result = tv_getBool((TValue *)vec_pop(vm->stack));
      if (!result) {
        VM_ERROR(msg);
      }
      break;
    }
    default:
      fprintf(stderr, "<VM error> Invalid op\n");
    }
  }
  return vm_stackPeekTop(vm);
}
#endif

void code_printer(Vector *code) {
  printf("=====================================================\n");
  for (int idx = 0; idx < code->len;) {
    int type = (int)code->data[idx++];
    switch (type) {
    case tOpVariableDeclareOnlySymbol:
    case tOpVariableDeclareWithAssign:
      type_print(type);
      printf(", ");
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    case tOpPop:
      type_print(type);
      printf("\n");
      break;
    case tOpPush:
      type_print(type);
      printf(", ");
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    case tOpAdd:
    case tOpSub:
    case tOpMul:
    case tOpDiv:
    case tOpMod:
    case tOpReturn:
      type_print(type);
      printf("\n");
      break;
    case tOpGetVariable:
    case tOpSetVariablePop:
    case tOpSetArrayElement:
    case tOpGetArrayElement:
    case tOpMakeArray:
    case tOpCall:
      type_print(type);
      printf(", ");
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    case tOpNop:
      type_print(type);
      printf("\n");
      break;
    case tOpFunctionDeclare:
      type_print(type);
      printf(", ");
      tv_print(code->data[idx++]);
      printf(", ");
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    case tOpEqualExpression:
    case tOpNotEqualExpression:
    case tOpLtExpression:
    case tOpLteExpression:
    case tOpGtExpression:
    case tOpGteExpression:
    case tOpAndExpression:
    case tOpOrExpression:
    case tOpXorExpression:
      type_print(type);
      printf("\n");
      break;
    case tOpJumpRel:
    case tOpJumpAbs:
      type_print(type);
      printf(", ");
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    case tOpPrint:
    case tOpPrintln:
      type_print(type);
      printf("\n");
      break;
    case tOpIFStatement:
    case tOpAssignExpression:
      type_print(type);
      printf(", ");
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    case tOpAssert:
      type_print(type);
      printf("\n");
      break;
    case tIValue:
      tv_print(code->data[idx++]);
      printf("\n");
      break;
    }
  }
  printf("=====================================================\n");
}