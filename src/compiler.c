/*
 * conditional.c
 *
 *  Created on: Oct 1, 2024
 *      Author: Jan Atle Ramsli
 *
 */
#include "compiler.h"
#include "logg.h"
#include "smtok.h"

#define DEBUG

program_p program_stack[32];
int program_Stack_top = 0;

void ct_prog_push(program_p prog)
{
	logg("NEW", prog->name);
	program_stack[program_Stack_top] = prog;
	program_Stack_top++;
}

program_p ct_prog_pop()
{
	program_Stack_top--;
	logg(prog->name,program_stack[program_Stack_top]->name);
	return program_stack[program_Stack_top];
}

static ct_t Ct_stack;
 
void cts_push(ctstack_t st,smtok_p cond)
{
	st.pca[st.npca] = cond;
	st.npca++;
}

smtok_p cts_pop(ctstack_t st)
{
	st.npca--;
	return st.pca[st.npca];
}

smtok_p cts_tos(ctstack_t st)
{
	return st.pca[st.npca - 1];
}


void ct_push(ct_stack_idx_t i,smtok_p cond)
{
	cts_push(Ct_stack.ct[i], cond);
}

smtok_p ct_pop(ct_stack_idx_t i)
{
	return cts_pop(Ct_stack.ct[i]);
}

smtok_p ct_tos(ct_stack_idx_t i)
{
	return cts_tos(Ct_stack.ct[i]);
}


/**
IF semantics: If TRUE, jump to offset in pcode.val.l 
IF FALSE, fall through 
**/
