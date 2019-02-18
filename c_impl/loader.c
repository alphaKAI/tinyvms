#include "sds/sds.h"
#include "tinyvm.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  Vector *array;
  int idx;
} TValueDeserializeResult;

TValueDeserializeResult *new_TValueDeserializeResult(Vector *array, int idx) {
  TValueDeserializeResult *ret = xmalloc(sizeof(TValueDeserializeResult));
  ret->array = array;
  ret->idx = idx;
  return ret;
}

TValueDeserializeResult *deserialize_to_tva(Vector *serialized,
                                            long long int elem_size) {
  Vector *deserialized = new_vec();
  long long int idx = 0;

  for (; idx < serialized->len && idx < elem_size; idx++) {
    // Emit type of elem
    int vtype = (int)serialized->data[idx++];
    switch (vtype) {
    case Long:
      vec_push(deserialized,
               new_TValue_with_integer((long long int)serialized->data[idx++]));
      break;
    case String: {
      long long int len = (long long int)serialized->data[idx++];
      char *buf = xmalloc(sizeof(char) * len);

      for (int i = 0; i < len; i++) {
        buf[i] = (char)serialized->data[idx++];
      }

      sds sb = sdsnewlen(buf, len);
      free(buf);

      vec_push(deserialized, new_TValue_with_str(sb));
      break;
    }
    case Bool:
      vec_push(deserialized,
               new_TValue_with_bool((bool)serialized->data[idx++]));
      break;
    case Array: {
      long long int len = (long long int)serialized->data[idx++];
      Vector *vec = new_vec();
      for (long long int i = idx; i < serialized->len; i++) {
        vec_push(vec, serialized);
      }

      TValueDeserializeResult *tvdr = deserialize_to_tva(vec, len);
      Vector *arr = tvdr->array;
      idx += tvdr->idx;
      vec_push(deserialized, new_TValue_with_array(arr));
      break;
    }
    case Function:
      fprintf(stderr, "Unsupported\n");
      exit(EXIT_FAILURE);
    case Null:
      vec_push(deserialized, new_TValue());
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

  for (long long int i = *idx; i < serialized->len; i++) {
    vec_push(vec, serialized->data[i]);
  }

  TValueDeserializeResult *tvdr = deserialize_to_tva(vec, 1);
  Vector *arr = tvdr->array;
  for (long long int i = 0; i < arr->len; i++) {
    vec_push(code, arr->data[i]);
  }
  *idx += tvdr->idx - 1;
}

void procWith2Arg(Vector *code, Vector *serialized, int type,
                  long long int *idx) {
  vec_pushi(code, type);
  /* first arg */
  /* vec = serialized[idx..$] */
  Vector *vec = new_vec();
  for (long long int i = *idx; i < serialized->len; i++) {
    vec_push(vec, serialized->data[i]);
  }

  TValueDeserializeResult *tvdr = deserialize_to_tva(vec, 1);
  Vector *arr = tvdr->array;
  for (long long int i = 0; i < arr->len; i++) {
    vec_push(code, arr->data[i]);
  }
  *idx += tvdr->idx - 1;

  /* second arg */
  /* vec = serialized[idx..$] */
  vec = new_vec();
  for (int i = *idx; i < serialized->len; i++) {
    vec_push(vec, serialized->data[i]);
  }

  tvdr = deserialize_to_tva(vec, 1);
  arr = tvdr->array;
  for (int i = 0; i < arr->len; i++) {
    vec_push(code, arr->data[i]);
  }
  *idx += tvdr->idx - 1;
}

Vector *deserialize(Vector *serialized) {
  Vector *code = new_vec();

  for (long long int idx = 0; idx < serialized->len;) {
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

Vector *readFromFile(char *filename) {
  FILE *fp;

  fp = fopen(filename, "rb");

  if (fp == NULL) {
    fprintf(stderr, "Failed to open the file - %s\n", filename);
    exit(EXIT_FAILURE);
  }

  Vector *buf = new_vec();
  long long int v;
  while (fread(&v, sizeof(long long int), 1, fp)) {
    vec_push(buf, (void *)v);
  }

  printf("Loaded byte codes: ");
  printf("[");
  for (int i = 0; i < buf->len; i++) {
    if (i > 0) {
      printf(", ");
    }
    printf("%lld", (long long int)buf->data[i]);
  }
  printf("]\n");

  fclose(fp);

  return deserialize(buf);
}
