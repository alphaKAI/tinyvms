#ifndef __TINY_VM_INCLUDE_GUARD__
#define __TINY_VM_INCLUDE_GUARD__

#include "sds/sds.h"
#include <stdbool.h>
#include <stddef.h>

//////////////////    Vector     //////////////////

typedef struct {
  void **data;
  long long int capacity;
  long long int len;
} Vector;

Vector *new_vec(void);
void free_vec(Vector **v_ptr);
void vec_expand(Vector *v, long long int size);
void vec_push(Vector *v, void *elem);
void vec_pushi(Vector *v, int val);
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
bool vec_contains(Vector *v, void *elem);
bool vec_containss(Vector *v, sds key);
bool vec_union1(Vector *v, void *elem);
void *vec_get(Vector *v, long long int idx);
Vector *vec_dup(Vector *v);

//////////////////      Map      //////////////////

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

Map *new_map(void);
void free_map(Map **map_ptr);
void map_put(Map *map, sds key, void *val);
void map_puti(Map *map, sds key, int val);
void *map_get(Map *map, sds key);
int map_geti(Map *map, sds key, int default_);
bool map_exists(Map *map, sds key);

//////////////////    env    //////////////////

typedef struct TValue_t TValue;

typedef struct VariableStore {
  struct VariableStore *super;
  bool has_super;
  Vector *protecteds;
  Map *store;
} VariableStore;

typedef struct {
  VariableStore *vs;
} Env;

typedef struct {
  TValue *tv;
  VariableStore *vs;
} HasPtrResult;

VariableStore *new_vs();
VariableStore *new_vs_with_super(VariableStore *super);
HasPtrResult *new_HasPtrResult(TValue *tv, VariableStore *vs);
HasPtrResult *vs_has_ptr(VariableStore *vs, sds key);
bool vs_has(VariableStore *vs, sds key);
bool vs_superHas(VariableStore *vs, sds key);
HasPtrResult *vs_superHas_ptr(VariableStore *vs, sds key);
TValue *vs_get(VariableStore *vs, sds key);
void vs_def(VariableStore *vs, sds key, TValue *value);
void vs_set(VariableStore *vs, sds key, TValue *value);

Env *new_env();
Env *new_env_with_vs(VariableStore *vs);
Env *env_dup(Env *env);
TValue *env_get(Env *env, sds key);
void env_def(Env *env, sds key, TValue *value);
void env_set(Env *env, sds key, TValue *value);
bool env_has(Env *env, sds key);
HasPtrResult *env_has_ptr(Env *env, sds key);

//////////////////    value     ////////////////////

typedef struct {
  sds func_name;
  Vector *func_body;
  Env *env;
} VMFunction;

typedef union {
  sds str;
  long long int integer;
  bool boolean;
  Vector *array;
  VMFunction *func;
} Value;

struct TValue_t {
  Value value;
  int tt;
};

TValue *new_TValue();
TValue *new_TValue_with_tt(int tt);
TValue *new_TValue_with_integer(long long int value);
TValue *new_TValue_with_str(sds sb);
TValue *new_TValue_with_bool(bool value);
TValue *new_TValue_with_array(Vector *array);
TValue *new_TValue_with_func(VMFunction *func);
long long int tv_getLong(TValue *tv);
sds tv_getString(TValue *tv);
bool tv_getBool(TValue *tv);
Vector *tv_getArray(TValue *tv);
VMFunction *tv_getFunction(TValue *tv);
bool tv_equals(TValue *this, TValue *that);
int tv_cmp(TValue *this, TValue *that);
TValue *tv_dup(TValue *tv);

bool tv_lt(TValue *a, TValue *b);
bool tv_lte(TValue *a, TValue *b);
bool tv_gt(TValue *a, TValue *b);
bool tv_gte(TValue *a, TValue *b);
bool tv_and(TValue *a, TValue *b);
bool tv_or(TValue *a, TValue *b);

void tv_print(TValue *v);

VMFunction *new_VMFunction(sds func_name, Vector *func_body, Env *env);

VMFunction *vmf_dup(VMFunction *func);

//////////////////    others     //////////////////

void *xmalloc(size_t size);
void xfree(void *ptr);

// ValueType
enum { Long, String, Bool, Array, Function, Null };

// Opcode
enum {
  tOpVariableDeclareOnlySymbol,
  tOpVariableDeclareWithAssign,
  tOpPop,
  tOpPush,
  tOpAdd,
  tOpSub,
  tOpMul,
  tOpDiv,
  tOpMod,
  tOpReturn,
  tOpGetVariable,
  tOpSetVariablePop,
  tOpSetArrayElement,
  tOpGetArrayElement,
  tOpMakeArray,
  tOpCall,
  tOpNop,
  tOpFunctionDeclare,
  tOpEqualExpression,
  tOpNotEqualExpression,
  tOpLtExpression,
  tOpLteExpression,
  tOpGtExpression,
  tOpGteExpression,
  tOpAndExpression,
  tOpOrExpression,
  tOpXorExpression,
  tOpJumpRel,
  tOpJumpAbs,
  tOpPrint,
  tOpPrintln,
  tOpIFStatement,
  tOpAssignExpression,
  tOpAssert,
  tIValue
};

typedef long long int Opcode;

/////////////// loader ///////////////
Vector *deserialize(Vector *serialized);
Vector *readFromFile(sds filename);

///////////////   VM   ///////////////
typedef struct {
  Env *env;
  Vector *stack;
} VM;

VM *new_VM();
TValue *vm_execute(VM *vm, Vector *code);

void type_print(int type);
void code_printer(Vector *code);
#endif
