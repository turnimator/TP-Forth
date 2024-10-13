/*
 * ffs.h
 *
 *  Created on: Sep 18, 2024
 *      Author: atle
 */

#ifndef TASK_H_
#define TASK_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct p_code *p_code_p;
typedef struct program program_t, *program_p;

#define D_STACK_SIZE 32
#define R_STACK_SIZE 32
#define S_STACK_SIZE 32
#define LOOP_STACK_SIZE 16

typedef struct ftask ftask_t, *ftask_p; // Bite a chunk out of our tail

typedef void (*funcptr)(ftask_p);

typedef struct ftask {
  long d_stack[D_STACK_SIZE];
  int d_top;
  int v_stack[S_STACK_SIZE];
  int v_top;
  program_p r_stack[R_STACK_SIZE];
  int r_top;
  program_p program;
  int loop_upper[LOOP_STACK_SIZE];
  int lu_top;
  int loop_lower[LOOP_STACK_SIZE];
  int ll_top;
} ftask_t, *ftask_p;

ftask_p ftask_create(char *name);

#endif /* TASK_H_ */
