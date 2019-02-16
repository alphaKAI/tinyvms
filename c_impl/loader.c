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

void procWith1Arg(Vector *code, Vector *serialized, int type,
                  long long int *idx) {
  vec_pushi(code, type);
  /* vec = serialized[idx..$] */
  Vector *vec = new_vec();
  vec->capacity = serialized->capacity - *idx;
  vec->len = serialized->len - *idx;
  vec->data = serialized->data[*idx];

  TValueDeserializeResult *tvdr = deserialize_to_tva(vec, 1);
  TValueArray *arr = tvdr->array;
  for (int i = 0; i < arr->vec->len; i++) {
    vec_push(code, arr->vec->data[i]);
  }
  *idx += tvdr->idx - 1;
}

void procWith2Arg(Vector *code, Vector *serialized, int type,
                  long long int *idx) {
  vec_pushi(code, type);
  /* first arg */
  /* vec = serialized[idx..$] */
  Vector *vec = new_vec();
  vec->capacity = serialized->capacity - *idx;
  vec->len = serialized->len - *idx;
  vec->data = serialized->data[*idx];

  TValueDeserializeResult *tvdr = deserialize_to_tva(vec, 1);
  TValueArray *arr = tvdr->array;
  for (int i = 0; i < arr->vec->len; i++) {
    vec_push(code, arr->vec->data[i]);
  }
  *idx += tvdr->idx - 1;

  /* second arg */
  /* vec = serialized[idx..$] */
  vec = new_vec();
  vec->capacity = serialized->capacity - *idx;
  vec->len = serialized->len - *idx;
  vec->data = serialized->data[*idx];

  tvdr = deserialize_to_tva(vec, 1);
  arr = tvdr->array;
  for (int i = 0; i < arr->vec->len; i++) {
    vec_push(code, arr->vec->data[i]);
  }
  *idx += tvdr->idx - 1;
}

Vector *deserialize(Vector *serialized) {
  Vector *code = new_vec();

  for (long long int idx; idx < serialized->len;) {
    int type = (int)serialized->data[idx++];
    switch (type) {
    case tOpVariableDeclareOnlySymbol:
    case tOpVariableDeclareWithAssign:
      procWith1Arg(code, serialized, type, &idx);
      break;
    case tOpPop:
      fprintf(stderr, "<Deserialize> Not supported %d", type);
      exit(EXIT_FAILURE);
    case tOpPush:
      procWith1Arg(code, serialized, type, &idx);
      break;
    case tOpAdd:
    case tOpSub:
    case tOpMul:
    case tOpDiv:
    case tOpMod:
    case tOpReturn:
      vec_pushi(code, type);
      break;
    case tOpGetVariable:
    case tOpSetVariablePop:
    case tOpSetArrayElement:
    case tOpGetArrayElement:
    case tOpMakeArray:
    case tOpCall:
      procWith1Arg(code, serialized, type, &idx);
      break;
    case tOpNop:
      vec_pushi(code, type);
      idx++;
      break;
    case tOpFunctionDeclare:
      procWith2Arg(code, serialized, type, &idx);
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
      vec_pushi(code, type);
      break;
    case tOpJumpRel:
    case tOpJumpAbs:
      procWith1Arg(code, serialized, type, &idx);
      break;
    case tOpPrint:
    case tOpPrintln:
      vec_pushi(code, type);
      break;
    case tOpIFStatement:
    case tOpAssignExpression:
      procWith1Arg(code, serialized, type, &idx);
      break;
    case tOpAssert:
      vec_pushi(code, type);
      break;
    case tIValue:
      fprintf(stderr, "<Deserialize> Not supported %d", type);
      exit(EXIT_FAILURE);
    default:
      fprintf(stderr, "There is no default!\n");
      exit(EXIT_FAILURE);
    }
  }

  return code;
}