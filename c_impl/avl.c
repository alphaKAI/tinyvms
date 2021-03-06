#include "tinyvm.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

AVLNode *new_AVLNode(void *key, void *value) {
  AVLNode *this = xmalloc(sizeof(AVLNode));
  this->key = key;
  this->value = value;
  this->height = 1;
  this->size = 1;
  return this;
}

AVLTree *new_AVLTree() {
  AVLTree *this = xmalloc(sizeof(AVLTree));
  this->root = NULL;
  return this;
}

static void *find_impl(AVLNode *t, void *key, ELEM_COMPARE compare) {
  if (t == NULL) {
    return NULL;
  }

  int comp_result = compare(key, t->key);

  if (comp_result == 0) {
    return t->value;
  } else if (comp_result == -1) {
    return find_impl(t->left, key, compare);
  } else {
    return find_impl(t->right, key, compare);
  }
}

bool avl_exists(AVLTree *tree, void *key, ELEM_COMPARE compare) {
  return find_impl(tree->root, key, compare) != NULL;
}

void *avl_find(AVLTree *tree, void *key, ELEM_COMPARE compare) {
  return find_impl(tree->root, key, compare);
}

static int int_max(int a, int b) { return a > b ? a : b; }

static AVLNode *rotate(AVLNode *t, int l, int r, ELEM_COMPARE compare);

static AVLNode *balance(AVLNode *t, ELEM_COMPARE compare) {
  if (ht(t->right) - ht(t->left) < -1) {
    if (ht(t->left->right) - ht(t->left->left) > 0) {
      t->left = rotate(t->left, L, R, compare);
    }
    return rotate(t, R, L, compare);
  }

  if (ht(t->left) - ht(t->right) < -1) {
    if (ht(t->right->left) - ht(t->right->right) > 0) {
      t->right = rotate(t->right, R, L, compare);
    }
    return rotate(t, L, R, compare);
  }

  if (t != NULL) {
    t->height = int_max(ht(t->left), ht(t->right)) + 1;
    t->size = sz(t->left) + sz(t->right) + 1;
  }
  return t;
}

static AVLNode *get_child_by_LR(AVLNode *t, int lr) {
  switch (lr) {
  case L:
    return t->left;
  case R:
    return t->right;
  default:
    fprintf(stderr, "ERROR\n");
    exit(EXIT_FAILURE);
  }
}

static void set_child_by_LR(AVLNode *dst, int lr, AVLNode *src) {
  switch (lr) {
  case L:
    dst->left = src;
    break;
  case R:
    dst->right = src;
    break;
  }
}

static AVLNode *rotate(AVLNode *t, int l, int r, ELEM_COMPARE compare) {
  AVLNode *s = get_child_by_LR(t, r);
  set_child_by_LR(t, r, get_child_by_LR(s, l));
  set_child_by_LR(s, l, balance(t, compare));

  if (t != NULL) {
    t->size = sz(t->left) + sz(t->right) + 1;
  }
  if (s != NULL) {
    s->size = sz(s->left) + sz(s->right) + 1;
  }

  return balance(s, compare);
}

static AVLNode *insert_impl(AVLNode *t, AVLNode *x, ELEM_COMPARE compare) {
  if (t == NULL) {
    return x;
  }

  int comp_result = compare(x->key, t->key);

  if (comp_result == 0) {
    t->value = x->value;
  } else if (comp_result == -1) {
    t->left = insert_impl(t->left, x, compare);
  } else {
    t->right = insert_impl(t->right, x, compare);
  }
  t->size += 1;

  return balance(t, compare);
}

void avl_insert(AVLTree *tree, void *key, void *value, ELEM_COMPARE compare) {
  tree->root = insert_impl(tree->root, new_AVLNode(key, value), compare);
}

static sds string_rep(sds s, size_t n) {
  sds ret = sdsempty();

  for (size_t i = 0; i < n; i++) {
    sdscat(ret, s);
  }

  return ret;
}

void print_node(AVLNode *node, size_t depth, ELEM_PRINTER key_printer,
                ELEM_PRINTER value_printer) {
  if (node != NULL) {
    print_node(node->left, depth + 1, key_printer, value_printer);
    printf("%s <%s:%s>\n", string_rep("    ", depth), key_printer(node->key),
           value_printer(node->value));
    print_node(node->right, depth + 1, key_printer, value_printer);
  }
}

void print_tree(AVLTree *tree, ELEM_PRINTER key_printer,
                ELEM_PRINTER value_printer) {
  print_node(tree->root, 0, key_printer, value_printer);
}
