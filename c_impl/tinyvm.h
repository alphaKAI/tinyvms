#ifndef __TINY_VM_INCLUDE_GUARD__
#define __TINY_VM_INCLUDE_GUARD__

#include <stdbool.h>
#include <stddef.h>

//////////////////    Vector     //////////////////

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vec(void);
void vec_push(Vector *v, void *elem);
void vec_pushi(Vector *v, int val);
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
bool vec_contains(Vector *v, void *elem);
bool vec_containss(Vector *v, char *key);
bool vec_union1(Vector *v, void *elem);
void *vec_get(Vector *v, int idx);

//////////////////      Map      //////////////////

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

Map *new_map(void);
void map_put(Map *map, char *key, void *val);
void map_puti(Map *map, char *key, int val);
void *map_get(Map *map, char *key);
int map_geti(Map *map, char *key, int default_);
bool map_exists(Map *map, char *key);

////////////////// StringBuilder //////////////////

typedef struct {
  char *data;
  int capacity;
  int len;
} StringBuilder;

StringBuilder *new_sb(void);
StringBuilder *new_sb_with_char(char *buf);
void sb_add(StringBuilder *sb, char c);
void sb_append(StringBuilder *sb, char *s);
void sb_append_n(StringBuilder *sb, char *s, int len);
char *sb_get(StringBuilder *sb);
StringBuilder *sb_dup(StringBuilder *sb);

//////////////////    env    //////////////////

typedef struct TValueArray_t TValueArray;
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
  TValue **tv;
  VariableStore *vs;
} HasPtrResult;

VariableStore *new_vs();
VariableStore *new_vs_with_super(VariableStore **super);
HasPtrResult *new_HasPtrResult(TValue **tv, VariableStore *vs);
HasPtrResult *vs_has_ptr(VariableStore *vs, StringBuilder *key);
bool vs_has(VariableStore *vs, StringBuilder *key);
bool vs_superHas(VariableStore *vs, StringBuilder *key);
HasPtrResult *vs_superHas_ptr(VariableStore *vs, StringBuilder *key);
TValue *vs_get(VariableStore *vs, StringBuilder *key);
void vs_def(VariableStore *vs, StringBuilder *key, TValue *value);
void vs_set(VariableStore *vs, StringBuilder *key, TValue *value);

Env *new_env();
Env *new_env_with_vs(VariableStore *vs);
Env *env_dup(Env *env);
TValue *env_get(Env *env, StringBuilder *key);
void env_def(Env *env, StringBuilder *key, TValue *value);
void env_set(Env *env, StringBuilder *key, TValue *value);
bool env_has(Env *env, StringBuilder *key);
HasPtrResult *env_has_ptr(Env *env, StringBuilder *key);

//////////////////    value     ////////////////////

typedef struct {
  StringBuilder *func_name;
  Vector *func_body;
  Env *env;
} VMFunction;

struct TValueArray_t {
  Vector *vec;
};

typedef union {
  StringBuilder *str;
  long long int integer;
  bool boolean;
  TValueArray *array;
  VMFunction *func;
} Value;

struct TValue_t {
  Value value;
  int tt;
};

TValue *new_TValue();
TValue *new_TValue_with_tt(int tt);
TValue *new_TValue_with_integer(long long int value);
TValue *new_TValue_with_str(StringBuilder *sb);
TValue *new_TValue_with_bool(bool value);
TValue *new_TValue_with_array(TValueArray *array);
TValue *new_TValue_with_func(VMFunction *func);
long long int tv_getLong(TValue *tv);
StringBuilder *tv_getString(TValue *tv);
bool tv_getBool(TValue *tv);
TValueArray *getArray(TValue *tv);
VMFunction *tv_getFunction(TValue *tv);
bool tv_equals(TValue *this, TValue *that);
int vs_cmp(TValue *this, TValue *that);
TValue *tv_dup(TValue *tv);

TValueArray *new_TValueArray();
void tva_push(TValueArray *array, TValue *elem);
TValue *tva_get(TValueArray *array, int idx);
TValueArray *tva_dup(TValueArray *array);

VMFunction *new_VMFunction(StringBuilder *func_name, Vector *func_body,
                           Env *env);

VMFunction *vmf_dup(VMFunction *func);

//////////////////    others     //////////////////

void *xmalloc(size_t size);

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
Vector *readFromFile(char *filename);

///////////////   VM   ///////////////
typedef struct {
  Env *env;
  Vector *stack;
} VM;

VM *new_VM();
TValue *vm_execute(VM *vm, Vector *code);

#endif
