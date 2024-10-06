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

typedef int (*exec_func)(p_code_p, ftask_p, int);

static inline int ef_error(p_code_p pcp, ftask_p task, int pcnt) {
  logg(pcp->name, "P-CODE ERROR");
  return pcnt + 1;
}

static inline int ef_number(p_code_p pcp, ftask_p task, int pcnt) {
  d_push(task, pcp->val.l);
  return pcnt + 1;
}

static inline int ef_primitive(p_code_p pcp, ftask_p task, int pcnt) {
  DB_builtins[pcp->val.pp->op]->code(task);
  return pcnt + 1;
}

static inline int ef_variable(p_code_p pcp, ftask_p task, int pcnt) {
  v_push(task, pcp->val.var_idx);
  return pcnt + 1;
}

/*
Check ToS, if TRUE, execute the code that follows.
If FALSE, jump to code after THEN (stored in pcp->val.l)
*/
static inline int ef_if(p_code_p pcp, ftask_p task, int pcnt) {
  if (d_pop(task) == 0) {
    // printf("IF CODE RUNS, FALL TO %d\n", pcnt + 1);
    return pcnt + 1;
  } else {
    // printf("IF CODE DOES NOT RUN, SKIP TO %ld\n", pcp->val.l + 1);
    return pcp->val.l + 1;
  }
  return 0;
}

/**
Look up the dictionary entry.
If found, run its program.
*/
static inline int ef_dict_entry(p_code_p pcp, ftask_p task, int pcnt) {
  dict_entry_p sub = 0;
  sub = dict_lookup(0, pcp->name);
  if (!sub) {
    printf("\nEXEC: %s dict_lookup failed!", pcp->name);
    return pcnt + 1;
  }
  run_prog(task, sub->prog);
  return pcnt + 1;
}

int while_loop_test_cb(ftask_p task) {
	return 0;
}

int do_loop_test_cb(ftask_p task) {
  long a = d_tos(task);
  long b = d_pick(task, 1);
  if (a >= b) {
    d_pop(task);
    d_pop(task);
    logg("TEST TRUE", buf);
    return 1;
  }
  d_pock(task, 0, a + 1);
  return 0;
}

/**
a
b
while a < b
    execute code that follows
  endwhile
  jump to code following LOOP  (stored in pcp->val.l)
*/
static inline int ef_loop_begin(p_code_p pcp, int (*tf)(ftask_p), ftask_p task,
                                int pcnt) {
  char buf[32];
  sprintf(buf, "Lno:%d %s: %ld", pcnt, pcp->name, d_tos(task));
  logg("TESTING", buf);
  if (tf(task)) {
    return pcp->val.l + 1; // Points to end of loop
  }
  sprintf(buf, " LOOP %d", pcnt);
  logg("TEST FALSE", buf);
  return pcnt + 1;
}

static inline int ef_do(p_code_p pcp, ftask_p task, int pcnt) {
  return ef_loop_begin(pcp, do_loop_test_cb, task, pcnt);
}
/**
jump to code at DO
*/
static inline int ef_loop_end(p_code_p pcp, ftask_p task, int pcnt) {
  char buf[32];
  sprintf(buf, "lno: %d->%ld", pcnt, pcp->val.l + 1);
  logg("LOOPING", buf);
  return pcp->val.l + 1;
}

/*
This is kind of hairy, just make sure that the place in the array farray[]
corresponds to the PCODE type. from p_code.h: PCODE_ERROR=0, PCODE_INTERNAL=1,
  PCODE_NUMBER=2,
  PCODE_VARIABLE=3,
  PCODE_DICT_ENTRY=4,
  PCODE_IF=5,
  PCODE_DO=6,
  PCODE_LOOP=7,
  PCODE_LAST*/
static exec_func farray[] = {ef_error,      ef_primitive,  ef_number,
                             ef_variable,   ef_dict_entry, ef_if,
                             ef_do, ef_loop_end};

static inline int p_code_exec_direct(p_code_p pcp, ftask_p task, int pcnt) {
  char buf[128];
  sprintf(buf, "%d %s%d", pcnt, pcp->name, pcp->type);
  logg("EXEC", buf);
  if (!pcp) {
    printf("\nEXEC: NULL p_code!");
    return pcnt + 1;
  }
  if (!pcp->name) {
    printf("\nEXEC: NULL p_code->name!");
    return pcnt + 1;
  }
  return farray[pcp->type](pcp, task, pcnt);
}

/**
Loop through the program, calling func for each p-code
*/
static int exec_loop(program_p progp, exec_func func, ftask_p vp) {
  for (int i = 0; i < progp->npcp_array;) {
    i = func(progp->pcp_array[i], vp, i);
  }
  return 0;
}
void run_prog(ftask_p task, program_p prog) {
  // printf("\nRUNNING:\n");
  // program_dump(prog);
  exec_loop(prog, p_code_exec_direct, task);
}

void run_task(ftask_p task) { run_prog(task, task->program); }
