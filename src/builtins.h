/*
 * primitives.h
 *
 *  Created on: Sep 18, 2024
 *      Author: Jan Atle Ramsli
 */

#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "task.h"
/**
For every value here, up to but not including last_opcode there MUST be an entry
in primitives_build_db() like the one below.
DB_Primitives[D_DUP] = primitive_create("DUP", D_DUP, d_dup);
**/


typedef struct built_in {
  char *name;
  int op;	// Index into  DB_builtin = "opcode"
  funcptr code;
} builtin_t, *builtin_p;

typedef struct idx_built_in {
  char *name;
  int op;
} idx_builtin_t, *idx_builtin_p;


extern builtin_p *DB_builtins;

void builtin_build_db(void);
void builtin_test(ftask_p task);
void builtin_add(char *name, funcptr code);
idx_builtin_p builtin_lookup(char *key);
void dump_idx_builtin(void);
void dump_builtin_db(void);

void d_push(ftask_p task, long val);
long d_pop(ftask_p task);
long d_tos(ftask_p task);
long d_pick(ftask_p task, int num);
void d_pock(ftask_p task, int no, long val);

 void v_push(ftask_p task, int idx);

#endif /* BUILTINS_H_ */
