#include "tinyvm.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  TValueArray *array;
  int idx;
} TValueDeserializeResult;

TValueDeserializeResult *new_TValueDeserializeResult(TValueArray *array,
                                                     int idx) {
  TValueDeserializeResult *ret = xmalloc(sizeof(TValueDeserializeResult));
  ret->array = array;
  ret->idx = idx;
  return ret;
}

TValueDeserializeResult *deserialize_to_tva(Vector *serialized,
                                            long long int elem_size) {
  TValueArray *deserialized = new_TValueArray();
  long long int idx = 0;

  for (; idx < serialized->len && idx < elem_size; idx++) {
    // Emit type of elem
    int vtype = (int)serialized->data[idx++];
    switch (vtype) {
    case Long:
      tva_push(deserialized,
               new_TValue_with_integer((long long int)serialized->data[idx++]));
      break;
    case String: {
      long long int len = (long long int)serialized->data[idx++];
      StringBuilder *sb = new_sb();
      for (int i = 0; i < len; i++) {
        sb_add(sb, (char)serialized->data[idx++]);
      }
      tva_push(deserialized, new_TValue_with_str(sb));
      break;
    }
    case Bool:
      tva_push(deserialized,
               new_TValue_with_bool((bool)serialized->data[idx++]));
      break;
    case Array: {
      long long int len = (long long int)serialized->data[idx++];
      Vector *vec = new_vec();
      vec->capacity = serialized->capacity - idx;
      vec->len = serialized->len - idx;
      vec->data = serialized->data[idx];

      TValueDeserializeResult *tvdr = deserialize_to_tva(vec, len);
      TValueArray *arr = tvdr->array;
      idx += tvdr->idx;
      tva_push(deserialized, new_TValue_with_array(arr));
      break;
    }
    case Function:
      fprintf(stderr, "Unsupported\n");
      exit(EXIT_FAILURE);
    case Null:
      tva_push(deserialized, new_TValue());
      break;
    }
  }

  return new_TValueDeserializeResult(deserialized, idx);
}

/*
Opcode[] deserialize(SerializedType[] serialized) {
  Opcode[] code;

  void procWith1Arg(Opcode type, ref long long int idx) {
    code ~ = type;
    auto ret = deserialize(serialized[idx..$], 1);
    Opcode[] operands;
    foreach (operand; ret[0]) { operands ~ = cast(Opcode) operand; }
    code ~ = operands;
    idx += ret[1] - 1;
  }

  void procWith2Arg(Opcode type, ref long long int idx) {
    code ~ = type;

    auto ret = deserialize(serialized[idx..$], 1);
    Opcode[] operands;
    foreach (operand; ret[0]) { operands ~ = cast(Opcode) operand; }
    code ~ = operands;
    idx += ret[1] - 1;

    ret = deserialize(serialized[idx..$], 1);
    operands = [];
    foreach (operand; ret[0]) { operands ~ = cast(Opcode) operand; }
    code ~ = operands;
    idx += ret[1] - 1;
  }

  for (long long int idx; idx < serialized.length;) {
    OpcodeType type = opnum_to_opcode[serialized[idx++]];
    final switch (type) with(OpcodeType) {
    case tOpVariableDeclareOnlySymbol:
      procWith1Arg(opVariableDeclareOnlySymbol, idx);
      break;
    case tOpVariableDeclareWithAssign:
      procWith1Arg(opVariableDeclareWithAssign, idx);
      break;
    case tOpPop:
      throw new Error("<Deserialize> Not supported %s".format(type));
    case tOpPush:
      procWith1Arg(opPush, idx);
      break;
    case tOpAdd:
      code ~ = opAdd;
      break;
    case tOpSub:
      code ~ = opSub;
      break;
    case tOpMul:
      code ~ = opMul;
      break;
    case tOpDiv:
      code ~ = opMul;
      break;
    case tOpMod:
      code ~ = opMul;
      break;
    case tOpReturn:
      code ~ = opReturn;
      break;
    case tOpGetVariable:
      procWith1Arg(opGetVariable, idx);
      break;
    case tOpSetVariablePop:
      procWith1Arg(opSetVariablePop, idx);
      break;
    case tOpSetArrayElement:
      procWith1Arg(opSetArrayElement, idx);
      break;
    case tOpGetArrayElement:
      procWith1Arg(opGetArrayElement, idx);
      break;
    case tOpMakeArray:
      procWith1Arg(opMakeArray, idx);
      break;
    case tOpCall:
      procWith1Arg(opCall, idx);
      break;
    case tOpNop:
      code ~ = opNop;
      idx++;
      break;
    case tOpFunctionDeclare:
      procWith2Arg(opFunctionDeclare, idx);
      break;
    case tOpEqualExpression:
      code ~ = opEqualExpression;
      break;
    case tOpNotEqualExpression:
      code ~ = opNotEqualExpression;
      break;
    case tOpLtExpression:
      code ~ = opLtExpression;
      break;
    case tOpLteExpression:
      code ~ = opLteExpression;
      break;
    case tOpGtExpression:
      code ~ = opGtExpression;
      break;
    case tOpGteExpression:
      code ~ = opGteExpression;
      break;
    case tOpAndExpression:
      code ~ = opAndExpression;
      break;
    case tOpOrExpression:
      code ~ = opOrExpression;
      break;
    case tOpXorExpression:
      code ~ = opXorExpression;
      break;
    case tOpJumpRel:
      procWith1Arg(opJumpRel, idx);
      break;
    case tOpJumpAbs:
      procWith1Arg(opJumpAbs, idx);
      break;
    case tOpPrint:
      code ~ = opPrint;
      break;
    case tOpPrintln:
      code ~ = opPrintln;
      break;
    case tOpIFStatement:
      procWith1Arg(opIFStatement, idx);
      break;
    case tOpAssignExpression:
      procWith1Arg(opAssignExpression, idx);
      break;
    case tOpAssert:
      code ~ = opAssert;
      break;
    case tIValue:
      throw new Error("<Deserialize> Not supported %s".format(type));
    }
  }

  return code;
}*/