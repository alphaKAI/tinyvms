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

  func_tv = new_TValue_with_func(
      new_VMFunction(new_sb_with_char("print"), func_body, vm->env));
  env_def(vm->env, new_sb_with_char("print"), func_tv);

  /* println */
  func_body = new_vec();
  vec_pushi(func_body, tOpPrintln);

  func_tv = new_TValue_with_func(
      new_VMFunction(new_sb_with_char("println"), func_body, vm->env));
  env_def(vm->env, new_sb_with_char("println"), func_tv);

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

TValue *vm_execute(VM *vm, Vector *code) {
  for (size_t pc = 0; pc < code->len; pc++) {
    int op = (int)code->data[pc];
    // printf("op : %d\n", op);
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
    case tOpPop:
      vec_pop(vm->stack);
      break;
    case tOpAdd: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      VM_ASSERT0(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack, (void *)(a->value.integer + b->value.integer));
      break;
    }
    case tOpSub: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack, (void *)(a->value.integer - b->value.integer));
      break;
    }
    case tOpMul: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack, (void *)(a->value.integer * b->value.integer));
      break;
    }
    case tOpDiv: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack, (void *)(a->value.integer / b->value.integer));
      break;
    }
    case tOpMod: {
      TValue *a = (TValue *)vec_pop(vm->stack),
             *b = (TValue *)vec_pop(vm->stack);
      assert(a->tt == b->tt && a->tt == Long);
      vec_push(vm->stack, (void *)(a->value.integer % b->value.integer));
      break;
    }
    case tOpReturn:
      return vm_stackPeekTop(vm);
    case tOpGetVariable: {
      TValue *v = (TValue *)code->data[pc++ + 1];
      VM_ASSERT(v != NULL, "Execute Error on tOpGetVariable");
      HasPtrResult *ptr = env_has_ptr(vm->env, tv_getString(v));
      if (ptr->tv != NULL) {
        vec_push(vm->stack, *ptr->tv);
      } else {
        fprintf(stderr, "No such a variable %s", sb_get((tv_getString(v))));
        exit(EXIT_FAILURE);
      }
      break;
    }
    /*
    case tOpSetVariablePop:
      auto dst = (TValue *)code->data[pc++ + 1];
      auto v = (TValue *)vec_pop(vm->stack);
      this.env.set(dst.getString, v);
      break;
      */
    case tOpCall: {
      TValue *func = (TValue *)code->data[pc++ + 1];
      StringBuilder *fname = tv_getString(func);
      Env *cpyEnv = vm->env;
      vm->env = env_dup(tv_getFunction(env_get(vm->env, fname))->env);
      vm_execute(vm, tv_getFunction(env_get(cpyEnv, fname))->func_body);
      vm->env = cpyEnv;
      break;
    }
      /*
    case tOpNop:
      break;
    case tOpFunctionDeclare:
      auto symbol = (TValue *)code->data[pc++ + 1];
      string func_name = symbol.getString;
      auto op_blocks_length = (TValue *)code->data[pc++ + 1];
      Opcode[] func_body;
      foreach (_; 0..op_blocks_length.getLong) { func_body ~ = code[pc++ + 1]; }
      this.env.def(func_name,
                   new TValue *(new VMFunction(func_name, func_body, env.dup)));
      break;
    case tOpEqualExpression:
      TValue *a = (TValue *)vec_pop(vm->stack),
             b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new TValue *(a == b));
      break;
    case tOpNotEqualExpression:
      TValue *a = (TValue *)vec_pop(vm->stack),
             b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new TValue *(a != b));
      break;
    case tOpLtExpression:
      TValue *a = (TValue *)vec_pop(vm->stack),
             b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new TValue *(a < b));
      break;
    case tOpLteExpression:
      TValue *a = (TValue *)vec_pop(vm->stack),
             b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new TValue *(a <= b));
      break;
    case tOpGtExpression:
      TValue *a = (TValue *)vec_pop(vm->stack),
             b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new TValue *(a > b));
      break;
    case tOpGteExpression:
      TValue *a = (TValue *)vec_pop(vm->stack),
             b = (TValue *)vec_pop(vm->stack);
      vec_push(vm->stack, new TValue *(a >= b));
      break;
    case tOpAndExpression:
      bool a = (TValue *)vec_pop(vm->stack).getBool,
           b = (TValue *)vec_pop(vm->stack).getBool;
      vec_push(vm->stack, new TValue *(a && b));
      break;
    case tOpOrExpression:
      bool a = (TValue *)vec_pop(vm->stack).getBool,
           b = (TValue *)vec_pop(vm->stack).getBool;
      vec_push(vm->stack, new TValue *(a || b));
      break;
    case tOpXorExpression:
      throw new Error("Not implemented <%s>".format(op.type));
      */
    case tOpPrint: {
      TValue *v = (TValue *)vec_pop(vm->stack);
      if (v->tt == String) {
        printf("%s", sb_get(tv_getString(v)));
      } else {
        fprintf(stderr, "ADD OTHER FORMATS <OpPrint>\n");
        exit(EXIT_FAILURE);
      }
      break;
    }
    case tOpPrintln: {
      TValue *v = (TValue *)vec_pop(vm->stack);
      if (v->tt == String) {
        printf("%s\n", sb_get(tv_getString(v)));
      } else {
        fprintf(stderr, "ADD OTHER FORMATS <OpPrint>\n");
        exit(EXIT_FAILURE);
      }
      break;
    }
      /*
    case tOpJumpRel:
      auto v = (TValue *)code->data[pc++ + 1];
      pc += v.getLong;
      break;
    case tOpJumpAbs:
      auto v = (TValue *)code->data[pc++ + 1];
      pc = v.getLong;
      break;
    case tOpIFStatement:
      TValue *cond = (TValue *)vec_pop(vm->stack);
      bool condResult;
      switch (cond.vtype)
        with(ValueType) {
        case Long:
          condResult = cond.getLong != 0;
          break;
        case Bool:
          condResult = cond.getBool;
          break;
        case String:
          throw new Exception("Execute Error Invalid Condition <string>");
        case Array:
          throw new Exception("Execute Error Invalid Condition <array>");
        case Function:
          throw new Exception("Execute Error Invalid Condition <function>");
        case Null:
          condResult = false;
          break;
        }

      auto trueBlockLength = ((TValue *)code->data[pc++ + 1]).getLong;

      if (condResult) {
        break;
      } else {
        pc += trueBlockLength;
      }
      break;
    case tOpSetArrayElement:
      auto variable = ((TValue *)code->data[pc++ + 1]).getString;
      auto idx = (TValue *)vec_pop(vm->stack)().getLong;
      auto val = (TValue *)vec_pop(vm->stack);
      env.get(variable).setArrayElement(idx, val);
      break;
    case tOpGetArrayElement:
      auto variable = ((TValue *)code->data[pc++ + 1]).getString;
      auto idx = (TValue *)vec_pop(vm->stack)().getLong;
      vec_push(vm->stack, env.get(variable)[idx]);
      break;
    case tOpMakeArray:
      long array_size = ((TValue *)code->data[pc++ + 1]).getLong;
      TValue *[] array;
      array.length = array_size;
      foreach_reverse(i; 0..array_size) {
        array[i] = (TValue *)vec_pop(vm->stack)();
      }
      vec_push(vm->stack, new TValue *(array));
      break;
    case tTValue *:
      throw new Error("TValue* should not peek directly");
    case tOpAssert:
      auto msg = (TValue *)vec_pop(vm->stack)().getString;
      auto result = (TValue *)vec_pop(vm->stack)().getBool;
      if (!result) {
        throw new VMException(msg);
      }
      */
    default:
      fprintf(stderr, "<VM error> Invalid op\n");
    }
  }
  return vm_stackPeekTop(vm);
}