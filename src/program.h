/*
 * program.h
 *
 *  Created on: Sep 29, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "smtok.h"
#include "task.h"

#define R_STACK_SIZE 32

typedef struct program {
  char *name; // The name is either MAIN of name of subroutine,
              // Colon-definition, IF ELSE THEN, LOOP etc
  smtok_p *pcp_array;
  int npcp_array;
} program_t, *program_p;

program_p program_create(char *program_name);

void program_delete(program_p prog);
void program_add(program_p program, char *word);
void program_add_smtok(program_p prog, smtok_p pcp);
void program_loop(program_p progp, void (*func)(program_p, smtok_p*, ftask_p),
                  ftask_p task);
void program_dump(program_p prog, ftask_p);
void program_dump_cb(program_p, smtok_p* pcp, ftask_p); // for stepping
void d_stack_dump(ftask_p task);                        //
#endif                                                  /* PROGRAM_H_ */
