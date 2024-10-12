/*
 * fnf_task.c
 *
 *  Created on: Sep 18, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "task.h"
#include "program.h"

ftask_p ftask_create(char *name) {
  ftask_p rv = malloc(sizeof(ftask_t));
  rv->program = program_create(name);
  rv->r_top = 0;
  rv->d_top = 0;
  rv->v_top = 0;
  rv->lu_top = 0;
  rv->ll_top = 0;
  return rv;
}

void ftask_delete(ftask_p task) {
  program_delete(task->program);
  free(task);
}