/*
 * fnf_p_machine.h
 *
 *  Created on: Sep 21, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef RUNTIME_H_
#define RUNTIME_H_

#include "builtins.h"
#include "dictionary.h"

void run_prog(ftask_p, program_p);
void run_task(ftask_p);
void ef_spawn(program_p prog, ftask_p task);
void ef_exec(program_p prog, ftask_p task);
#endif /* RUNTIME_H_ */
