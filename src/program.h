/*
 * program.h
 *
 *  Created on: Sep 29, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "p_code.h"
#include "task.h"

typedef struct program {
  char *name; // The name is either MAIN of name of subroutine,
              // Colon-definition, IF ELSE THEN, LOOP etc
  p_code_p *pcp_array;
  int npcp_array;
  p_code_p* pcp; // Program counter pointer
} program_t, *program_p;

program_p program_create(char *program_name);
void program_delete(program_p prog);
void program_add(program_p program, char *word);
void program_add_p_code(program_p prog, p_code_p pcp);
int offs(program_p prog, p_code_p *p) ;
void program_loop(program_p progp, void (*func)(program_p, p_code_p, ftask_p), ftask_p task);
void program_dump(program_p prog, ftask_p);
p_code_p* program_last(program_p);
void program_dump_cb(program_p, p_code_p pcp, ftask_p); // for stepping
void d_stack_dump(ftask_p task); //
#endif /* PROGRAM_H_ */
