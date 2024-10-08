/*
 * fnf_p_machine.c
 *
 *  Created on: Sep 21, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "runtime.h"

#include "builtins.h"
#include "dictionary.h"
#include "logg.h"
#include "p_code.h"
#include "program.h"
#include "task.h"
extern int STEP;

typedef int (*exec_func)(program_p, ftask_p, int);
static exec_func farray[];

static inline int ef_error(program_p prog, ftask_p task, int pcnt) {
  logg(prog->pcp_array[pcnt]->name, "P-CODE ERROR");
  return pcnt + 1;
}

static inline int ef_number(program_p prog, ftask_p task, int pcnt) {
  d_push(task, prog->pcp_array[pcnt]->val.l);
  return pcnt + 1;
}

static inline int ef_primitive(program_p prog, ftask_p task, int pcnt) {
  DB_builtins[prog->pcp_array[pcnt]->val.pp->op]->code(task);
  return pcnt + 1;
}

static inline int ef_variable(program_p prog, ftask_p task, int pcnt) {
  v_push(task, prog->pcp_array[pcnt]->val.var_idx);
  return pcnt + 1;
}

/**
Look up the dictionary entry.
If found, run its program.
*/
static inline int ef_dict_entry(program_p prog, ftask_p task, int pcnt) {
  dict_entry_p sub = 0;
  sub = dict_lookup(0, prog->pcp_array[pcnt]->name);
  if (!sub) {
    printf("\nEXEC: %s dict_lookup failed!", prog->pcp_array[pcnt]->name);
    return pcnt + 1;
  }
  run_prog(task, sub->prog);
  return pcnt + 1;
}

/**
a
b
while a < b
    execute code that follows
  endwhile
  jump to code following LOOP  (stored in pcp->val.l)
*/

static int program_exec_word_no(program_p, ftask_p, int);

static inline int ef_i_cb(program_p prog, ftask_p task, int pcnt) {
  printf("\nERROR I called from farray index\n");
  return ++pcnt;
}

static int run_block(program_p progp, ftask_p task, int pcnt, int i) {
  while (progp->pcp_array[pcnt]->type != PCODE_LOOP_END) {
    if (progp->pcp_array[pcnt]->type == PCODE_I) { // This is a kludge
      d_push(task, i);
      pcnt++;
    } else {
      pcnt = program_exec_word_no(progp, task, pcnt);
    }
  }
  return pcnt;
}
/*
We may be running different programs in the same task.
Dictionary entries have their own program
*/
static inline int ef_do(program_p progp, ftask_p task, int pcnt) {
  long l1 = d_pop(task);
  long l2 = d_pop(task);

  int eol = progp->pcp_array[pcnt]->val.l;
  // printf("EOL=%d\n", eol);
  for (int i = 0; i < (l2 - l1); i++) {
   // printf("\nLoopcnt: %d pcnt:%d", i, pcnt);
   // program_dump(progp, pcnt);
    run_block(progp, task, pcnt + 1, i + l1);
  }
  return eol;
}
/**
jump to code at DO
*/
static inline int ef_loop_end(program_p progp, ftask_p task, int pcnt) {
  return (progp->pcp_array[pcnt]->val.l);
}


/*
Check ToS, if TRUE, execute the code that follows.
If FALSE, jump to code after ELSE or THEN (stored in pcp->val.l)
*/
static inline int ef_if(program_p prog, ftask_p task, int pcnt) {
  if (d_pop(task) == 0) {
    //printf("IF CODE TRUE, FALL THROUGH\n");
    return pcnt + 1;
  } else {
    //printf("IF CODE FALSE, SKIP TO PAST ELSE OR THEN%ld\n", prog->pcp_array[pcnt]->val.l);
    return prog->pcp_array[pcnt]->val.l + 1;
  }
  return 0;
}


static inline int ef_else(program_p progp, ftask_p task, int pcnt) {
	//printf("ELSE CAUGHT IT, SKIP TO THEN\n");
  return progp->pcp_array[pcnt]->val.l;
}

static inline int ef_then(program_p progp, ftask_p task, int pcnt) {
	//printf("THEN CAUGHT IT, FALLING THROUG\n");
  return pcnt + 1;
}

/*
This is kind of hairy, just make sure that the place in the array farray[]
corresponds to the PCODE type. from p_code.h:
   PCODE_ERROR = 0,
  PCODE_BUILTIN = 1,
  PCODE_NUMBER = 2,
  PCODE_VARIABLE = 3,
  PCODE_DICT_ENTRY = 4,
  PCODE_IF = 5,
  PCODE_LOOP_DO = 6,
  PCODE_LOOP_END = 7,
  PCODE_I = 8,
  PCODE_ELSE = 9,
  PCODE_THEN =10,
  PCODE_LAST*/
static exec_func farray[] = {
    ef_error, ef_primitive, ef_number, ef_variable, ef_dict_entry, ef_if,
    ef_do,    ef_loop_end,  ef_i_cb,   ef_else,     ef_then};

static inline int program_exec_word_no(program_p progp, ftask_p task,
                                       int pcnt) {
  char buf[128];
  sprintf(buf, "%d %s%d", pcnt, progp->name, progp->pcp_array[pcnt]->type);
  logg("EXEC", buf);

  if (STEP) {
    d_stack_dump(task);
    program_dump(progp, pcnt);
    printf("\n...");
    if (getchar() == 'x') {
      STEP = 0;
    }
  }
  return farray[progp->pcp_array[pcnt]->type](progp, task, pcnt);
}

/**
Loop through the program, calling func for each p-code
*/
static int program_exec_loop(program_p progp, exec_func func, ftask_p vp) {
  for (int i = 0; i < progp->npcp_array;) {
    i = func(progp, vp, i);
  }
  return 0;
}

void run_prog(ftask_p task, program_p prog) {
  if (STEP) {
    printf("\nRUNNING:\n");
    program_dump(prog, 0);
  }
  program_exec_loop(prog, program_exec_word_no, task);
}

void run_task(ftask_p task) { run_prog(task, task->program); }
