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
#include "p_code.h"

// You should never use more than 3 nested IFs, DOs or other control structures, here's 15
typedef struct ctstack {
	p_code_p pca[15]; 
	int npca;
} ctstack_t;

typedef enum ct_stack_idx {
	CT_IF,
	CT_ELSE,
	CT_DO,
	CT_LOOP
} ct_stack_idx_t;


typedef struct compiletime_stack_array {
	ctstack_t ct[5]; 
} ct_t;



void ct_push(ct_stack_idx_t,p_code_p cond);
p_code_p ct_pop(ct_stack_idx_t);
p_code_p ct_tos(ct_stack_idx_t);
p_code_p ct_create(p_code_type_t pctyp);
#endif /* COMPILER_H_ */
