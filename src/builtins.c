/*
 * builtins.c
 *
 *  Created on: Sep 18, 2024
 *      Author: atle
 */

#include "builtins.h"

#include "custom.h"
#include "dictionary.h"
#include "program.h"
#include "task.h"
#include "variable.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

builtin_p *DB_builtins;
int N_builtins = 0;

int STEP = 0;

idx_builtin_t *IDX_builtins = 0;

static void bdb_create() {
  DB_builtins = malloc(sizeof(builtin_p *));
  N_builtins = 0;
}

static void bdb_add(builtin_p bip) {
  bip->op = N_builtins;
  N_builtins++;
  DB_builtins = realloc(DB_builtins, sizeof(builtin_p *) * N_builtins);
  DB_builtins[bip->op] = bip;
}

void r_push(ftask_p task, p_code_p *p) {
  task->r_stack[task->r_top] = p;
  task->r_top++;
}

p_code_p *r_pop(ftask_p task) {
  task->r_top--;
  return task->r_stack[task->r_top];
}

p_code_p *r_tos(ftask_p task) { return task->r_stack[task->r_top - 1]; }

/**
Create a builtin
 */
builtin_p builtin_create(char *name, funcptr code) {
  builtin_p rv = malloc(sizeof(builtin_t));
  rv->name = name;
  rv->code = code;
  return rv;
}

inline void d_push(ftask_p task, long val) {
  task->d_stack[task->d_top] = val;
  task->d_top++;
}

inline void lu_push(ftask_p task, long val) {
  task->loop_upper[task->lu_top] = val;
  task->lu_top++;
}

inline void ll_push(ftask_p task, long val) {
  task->loop_lower[task->ll_top] = val;
  task->ll_top++;
}

inline long d_pop(ftask_p task) {
  if (task->d_top < 0) {
    printf("Data Stack Underflow -- press x to exit");
    if (getchar() == 'x') {
      exit(-1);
    }
    return 0;
  }
  long l = task->d_stack[task->d_top - 1];
  task->d_top--;
  return l;
}

inline long lu_pop(ftask_p task) {
  if (task->lu_top < 0) {
    printf("Upper Loop Stack Underflow");
    printf(" -- press x to exit");
    if (getchar() == 'x') {
      exit(-1);
    }
    return 0;
  }
  long l = task->loop_upper[task->lu_top - 1];
  task->lu_top--;
  return l;
}

/*
Using macros for these
inline long ll_tos(ftask_p task) { return task->loop_lower[task->ll_top - 1]; }

inline long lu_tos(ftask_p task) { return task->loop_upper[task->lu_top - 1]; }
 */

inline long ll_pop(ftask_p task) {
  if (task->ll_top < 0) {
    printf("Lower Loop Stack Underflow");
    printf(" -- press x to exit");
    if (getchar() == 'x') {
      exit(-1);
    }
    return 0;
  }
  long l = task->loop_lower[task->ll_top - 1];
  task->ll_top--;
  return l;
}

long d_pick(ftask_p task, int num) {
  return task->d_stack[task->d_top - (num + 1)];
}

/***
if PICK is modeled after PEEK, then POCK must be modeled after POKE, right?
 */
void d_pock(ftask_p task, int num, long val) {
  task->d_stack[task->d_top - (num + 1)] = val;
}

long d_tos(ftask_p task) { return task->d_stack[task->d_top - 1]; }

void v_push(ftask_p task, int i) {
  task->v_stack[task->v_top] = i;
  task->v_top++;
}

int v_pop(ftask_p task) {
  if (task->v_top <= 0) {
    printf("\n****VariableStack Underflow***");
    printf(" -- press x to exit");
    if (getchar() == 'x') {
      exit(-1);
    }
    return 0;
  }
  int i = task->v_stack[task->v_top - 1];
  task->v_top--;
  return i;
}

inline int v_tos(ftask_p task) { return task->v_stack[task->v_top - 1]; }

static inline void d_gt(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, (l2 > l1) ? 0 : 1);
}

static inline void d_step(ftask_p task) {
  int l1 = d_pop(task);
  if (l1 == 0) {
    STEP = 1;
    printf("\nPress x to exit stepping ...\n==>");
  } else {
    STEP = 0;
  }
}

static inline void d_and(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, (!l2 && !l1) ? 0 : 1);
}

static inline void d_or(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, (!l2 || !l1) ? 0 : 1);
}

static inline void d_not(ftask_p task) {
  long l1 = d_pop(task);

  d_push(task, (l1 == 0) ? 1 : 0);
}

static inline void d_lt(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, (l2 < l1) ? 0 : 1);
}

static inline void d_eq(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, (l2 == l1) ? 0 : 1);
}

static inline void d_true(ftask_p task) { d_push(task, 0); }

static inline void d_false(ftask_p task) { d_push(task, 1); }

void d_dup(ftask_p task) {
  task->d_stack[task->d_top] = task->d_stack[task->d_top - 1];
  task->d_top++;
}

static void d_plus(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, l1 + l2);
}

static void d_minus(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, l2 - l1);
}

static void d_mul(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, l2 * l1);
}

static void d_modulo(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, l2 % l1);
}

static void d_div(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, l2 / l1);
}

static void d_i(ftask_p task) {}

static void d_dot(ftask_p task) {
  if (task->d_top < 0) {
    printf("Stack Underflow");
    printf(" -- press x to exit");
    if (getchar() == 'x') {
      exit(-1);
    }
    return;
  }
  task->d_top--;
  printf("%ld ", task->d_stack[task->d_top]);
}

static void d_cr(ftask_p task) { printf("\n"); }

static inline void f_pick(ftask_p task) {
  long l = d_pick(task, d_pop(task));
  d_push(task, l);
}

static inline void d_swap(ftask_p task) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);
  d_push(task, l1);
  d_push(task, l2);
}

static inline void d_over(ftask_p task) {
  long l1 = d_pick(task, 1);
  d_push(task, l1);
}

static inline void d_rot(ftask_p task) {
  long l3 = d_pop(task);
  long l2 = d_pop(task);
  long l1 = d_pop(task);
  d_push(task, l2);
  d_push(task, l3);
  d_push(task, l1);
}

static inline void d_drop(ftask_p task) { task->d_top--; }

static inline void d_drop2(ftask_p task) { task->d_top -= 2; }

/**
Store value on data stack in variable on variable stack
 */
static void d_store_val(ftask_p task) {
  long val = d_pop(task);
  var_p v = variable_get(val);
  if (!v) {
    printf("Variable not found!\n");
  } else {
    v->val.l = val;
  }
}

static void d_load_val(ftask_p task) {
  var_p v = variable_get(d_pop(task));
  if (!v) {
    printf("Variable not found!\n");
  } else {
    d_push(task, v->val.l);
  }
}

static void s_dot(ftask_p task) {
  int idx = v_pop(task);
  var_p v = variable_get(idx);
  printf("%s %ld", v->name, v->val.l);
}

void d_stack_dump(ftask_p task) {
  printf("\n[");
  for (int i = 0; i < task->d_top; i++) {
    printf(" %ld", task->d_stack[i]);
  }
  printf(" ]\n");
}

static void f_exit(ftask_p task) {
  // Handled in runtime.c
}

void f_dict_dump(ftask_p task) { dict_dump(0); }

//// RETURN STACK WORDS ///////////

/*
data stack to return stack
*/
static void d_r(ftask_p task) { r_push(task, (p_code_p *)d_pop(task)); }

static void r_d(ftask_p task) { d_push(task, (long)r_pop(task)); }

inline void r_fetch(ftask_p task) {
  d_push(task, (long)task->r_stack[task->r_top - 1]);
}

///////////////////////////////////

void builtin_add(char *name, funcptr code) {
  bdb_add(builtin_create(name, code));
}

void builtins_test(ftask_p task) {}

static int qsort_builtins_compare(const void *a, const void *b) {
  idx_builtin_p pa = (idx_builtin_p)a;
  idx_builtin_p pb = (idx_builtin_p)b;
  return strcmp(pa->name, pb->name);
}

idx_builtin_p builtin_lookup(char *key) {
  int hi = N_builtins - 1;
  int lo = 0;

  //	printf("lookup(%s)\n", key);

  for (lo = 0, hi = N_builtins - 1; hi > lo;) {
    int i = (hi + lo) >> 1;
    int cmpval = strcmp(IDX_builtins[i].name, key);
    if (cmpval == 0) {
      return IDX_builtins + i;
    }
    if (hi - lo <= 1) {
      return 0;
    }
    if (cmpval < 0) {
      lo = i;
    } else if (cmpval > 0) {
      hi = i;
    }
  }
  return 0;
}

void index_names() {
  if (IDX_builtins) {
    IDX_builtins = realloc(IDX_builtins, sizeof(idx_builtin_t) * (N_builtins));
  } else {
    IDX_builtins = malloc(sizeof(idx_builtin_t) * (N_builtins));
  }
  for (int i = 0; i < N_builtins; i++) {
    IDX_builtins[i].name = DB_builtins[i]->name;
    IDX_builtins[i].op = DB_builtins[i]->op;
  }
  qsort(IDX_builtins, N_builtins, sizeof(idx_builtin_t),
        qsort_builtins_compare);
}

void builtin_db_dump() {
  printf("\nDB_builtins\n----------------------------\n");
  for (int i = 0; i < N_builtins; i++) {
    printf("%s\t%d %lx\n", DB_builtins[i]->name, DB_builtins[i]->op,
           (unsigned long)DB_builtins[i]->code);
  }
  printf("----------------------------\n");
}

void idx_builtins_dump() {
  printf("\nIDX\n----------------------------\n");
  for (int i = 0; i < N_builtins; i++) {
    printf("%s\t%d\n", IDX_builtins[i].name, IDX_builtins[i].op);
  }
  printf("----------------------------\n");
}

/*
Put all the builtins in the place indicated by their opcode.
There must be one entry here for each opcode.
If one is missing, the system will crash on qsort().
 */
void builtin_build_db() {
  bdb_create();
  builtin_add("DUP", d_dup);
  builtin_add("SWAP", d_swap);
  builtin_add("OVER", d_over);
  builtin_add("ROT", d_rot);
  builtin_add("DROP", d_drop);
  builtin_add("2DROP", d_drop2);
  builtin_add("PICK", f_pick);
  builtin_add("+", d_plus);
  builtin_add("-", d_minus);
  builtin_add(".", d_dot);
  builtin_add("AND", d_and);
  builtin_add("OR", d_or);
  builtin_add("NOT", d_not);
  builtin_add("CR", d_cr);
  builtin_add("s.", s_dot);
  builtin_add(".s", d_stack_dump);
  builtin_add(">", d_gt);
  builtin_add("<", d_lt);
  builtin_add("=", d_eq);
  builtin_add("*", d_mul);
  builtin_add("MOD", d_modulo);
  builtin_add("%", d_modulo);
  builtin_add("/", d_div);
  builtin_add("TRUE", d_true);
  builtin_add("FALSE", d_false);
  builtin_add("I", d_i);
  builtin_add("!", d_store_val);
  builtin_add("@", d_load_val);
  builtin_add("STEP", d_step);
  builtin_add("EXIT", f_exit);
  builtin_add("DICT", f_dict_dump);
  builtin_add(">r", d_r);
  builtin_add("r>", r_d);
  add_custom_builtins();
  //    builtin_db_dump();
  index_names();
  //    idx_builtins_dump();
}
