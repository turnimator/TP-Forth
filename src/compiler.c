/*
 * conditional.c
 *
 *  Created on: Oct 1, 2024
 *      Author: Jan Atle Ramsli
 *
 */
#include "compiler.h"
#include "p_code.h"

program_p program_stack[32];
int program_Stack_top = 0;

void ct_prog_push(program_p prog)
{
	program_stack[program_Stack_top] = prog;
	program_Stack_top++;
}

program_p ct_prog_pop()
{
	program_Stack_top--;
	return program_stack[program_Stack_top];
}

static ct_t Ct_stack;
 
void cts_push(ctstack_t st,p_code_p cond)
{
	st.pca[st.npca] = cond;
	st.npca++;
}

p_code_p cts_pop(ctstack_t st)
{
	st.npca--;
	return st.pca[st.npca];
}

p_code_p cts_tos(ctstack_t st)
{
	return st.pca[st.npca - 1];
}


void ct_push(ct_stack_idx_t i,p_code_p cond)
{
	cts_push(Ct_stack.ct[i], cond);
}

p_code_p ct_pop(ct_stack_idx_t i)
{
	return cts_pop(Ct_stack.ct[i]);
}

p_code_p ct_tos(ct_stack_idx_t i)
{
	return cts_tos(Ct_stack.ct[i]);
}


/**
IF semantics: If TRUE, jump to offset in pcode.val.l 
IF FALSE, fall through 
**/
