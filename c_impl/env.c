#include "sds/sds.h"
#include "tinyvm.h"

VariableStore *new_vs() {
  VariableStore *vs = xmalloc(sizeof(VariableStore));

  vs->super = NULL;
  vs->has_super = false;
  vs->protecteds = new_vec();
  vs->store = new_map();

  return vs;
}

VariableStore *new_vs_with_super(VariableStore *super) {
  VariableStore *vs = new_vs();
  vs->super = super;
  vs->has_super = true;
  return vs;
}

HasPtrResult *new_HasPtrResult(TValue *tv, VariableStore *vs) {
  HasPtrResult *hpr = xmalloc(sizeof(HasPtrResult));
  hpr->tv = tv;
  hpr->vs = vs;
  return hpr;
}

HasPtrResult *vs_has_ptr(VariableStore *vs, sds key) {
  HasPtrResult *ret = new_HasPtrResult(map_get(vs->store, key), vs);

  if (ret->tv == NULL && vs->has_super) {
    ret = vs_has_ptr(vs->super, key);
  }

  return ret;
}

/**
 * 現在のインスタンス，もしくは親に，keyに対応するIValueが存在するかを判定する
 */
bool vs_has(VariableStore *vs, sds key) {
  bool ret = map_get(vs->store, key) != NULL ? true : false;

  if (vs->has_super) {
    ret |= vs_has(vs->super, key);
  }

  return ret;
}

/**
 * superクラスのstoreにkeyに対応するIValueが存在するかを判定する
 */
bool vs_superHas(VariableStore *vs, sds key) {
  if (vs->has_super) {
    return vs_has(vs->super, key);
  } else {
    return false;
  }
}

HasPtrResult *vs_superHas_ptr(VariableStore *vs, sds key) {
  if (vs->has_super) {
    return vs_has_ptr(vs->super, key);
  } else {
    return new_HasPtrResult(NULL, NULL);
  }
}

TValue *vs_get(VariableStore *vs, sds key) {
  // 親が存在するかを判定する
  if (vs->has_super) {
    // 保護されている場合，このインスタンスのstoreから参照する
    if (vec_containss(vs->protecteds, key)) {
      return map_get(vs->store, key);
    } else {
      // 親がkeyを持っているかみる
      HasPtrResult *ptr = vs_superHas_ptr(vs, key);
      if (ptr->tv != NULL) {
        // 持っている場合，親から参照する
        return ptr->tv;
      } else {
        // 持っていない場合，現在のインスタンスから参照する．
        return map_get(vs->store, key);
      }
    }
  } else {
    // 親が存在しないので，現在のインスタンスから参照する．
    return map_get(vs->store, key);
  }
}

/**
 * 変数を定義する．
 * 定義したスコープで保護するようにする．(親の変数を触らなくする)
 */
void vs_def(VariableStore *vs, sds key, TValue *value) {
  // 親がいるか判定する．
  if (vs->has_super) {
    // 親がいる場合，親がkeyを持っているかを判定する
    if (vs_superHas(vs, key)) {
      // 持っている場合，保護対象として，protectedsに追加し，保存する．
      vec_push(vs->protecteds, key);
      map_put(vs->store, key, value);
    } else {
      // 持っていない場合，何もせずに現在のインスタンスに保存する
      map_put(vs->store, key, value);
    }
  } else {
    // 親がいない場合は現在のインスタンスのstoreに保存する．
    map_put(vs->store, key, value);
  }
}

void vs_set(VariableStore *vs, sds key, TValue *value) {
  // 親が存在するかの確認
  if (vs->has_super) { //存在する
    // 保護されている(つまり，現在のスコープで定義されている場合)場合は親を書き換えてはいけないので
    // このインスタンスのstoreを書き換える．
    if (vec_containss(vs->protecteds, key)) {
      map_put(vs->store, key, value);
    } else {
      // 保護されていない場合，親がkeyを持っているかをみる．
      HasPtrResult *ptr = vs_superHas_ptr(vs, key);
      if (ptr->tv != NULL) {
        // 親がkeyを持っている場合，親にsetさせる．
        vs_set(ptr->vs, key, value);
      } else {
        // 親がkeyを持っていない場合，自分のstoreを書き換える．
        map_put(vs->store, key, value);
      }
    }
  } else {
    // 存在しないなら現在のstoreを変更して良い
    map_put(vs->store, key, value);
  }
}

// Make a new Env
Env *new_env() {
  Env *env = xmalloc(sizeof(Env));
  env->vs = new_vs();
  return env;
}

// Make a new Env with VariableStore
Env *new_env_with_vs(VariableStore *vs) {
  Env *env = new_env();
  env->vs = new_vs_with_super(vs);
  return env;
}

// Duplicate an Env
Env *env_dup(Env *env) { return new_env_with_vs(env->vs); }

TValue *env_get(Env *env, sds key) { return vs_get(env->vs, key); }

void env_def(Env *env, sds key, TValue *value) { vs_def(env->vs, key, value); }

void env_set(Env *env, sds key, TValue *value) { vs_set(env->vs, key, value); }

bool env_has(Env *env, sds key) { return vs_has(env->vs, key); }

HasPtrResult *env_has_ptr(Env *env, sds key) {
  return vs_has_ptr(env->vs, key);
}