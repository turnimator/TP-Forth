/*
 * compiler.h
 *
 *  Created on: Oct 1, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef COMPILER_H_
#define COMPILER_H_
#include "builtins.h"
#include "dictionary.h"
#include "smtok.h"

// You should never use more than 3 nested IFs, DOs or other control structures,
// here's 15
typedef struct ctstack {
  smtok_p pca[15];
  int npca;
} ctstack_t;

typedef enum ct_stack_idx { CT_IF, CT_DO, CT_BEGIN } ct_stack_idx_t;

typedef struct compiletime_stack_array {
  ctstack_t ct[5];
} ct_t;

void ct_prog_push(program_p prog);
program_p ct_prog_pop();

void ct_push(ct_stack_idx_t, smtok_p cond);
smtok_p ct_pop(ct_stack_idx_t);
smtok_p ct_tos(ct_stack_idx_t);
smtok_p ct_create(jumptable_idx_t pctyp);


#endif /* COMPILER_H_ */
