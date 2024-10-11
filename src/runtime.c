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

typedef void (*cbp_exec_func)(program_p, ftask_p);
static cbp_exec_func farray[];

static void ef_error(program_p prog, ftask_p task) {
  logg(task->pcp->name, "P-CODE ERROR");
  task->pcp = program_last(prog);
}

static inline void cb_ef_number(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  d_push(task, pcp->val.l);
  task->pcp++;
}

static inline void ef_primitive(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  DB_builtins[pcp->val.pp->op]->code(task);
  task->pcp++;
}

static inline void ef_variable(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  v_push(task, pcp->val.var_idx);
  task->pcp++;
}

/**
Look up the dictionary entry.
If found, run its program.
*/
static inline void ef_dict_entry(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  dict_entry_p sub = 0;
  sub = dict_lookup(0, pcp->name);
  if (!sub) {
    printf("\nEXEC: %s dict_lookup failed!", pcp->name);
    task->pcp++;
  }
  run_prog(task, sub->prog);
  task->pcp++;
}

/**
a
b
while a < b
    execute code that follows
  endwhile
  jump to code following LOOP  (stored in pcp->val.l)
*/

static void cb_program_exec_word(program_p, ftask_p);

static inline void ef_i_cb(program_p prog, ftask_p task) {
  d_push(task, task->loop_lower);
  task->pcp++;
}

static void run_block(program_p progp, ftask_p task, int i) {
  p_code_p pcp = *task->pcp;
  while (pcp->type != PCODE_LOOP_END) {
    if (pcp->type == PCODE_I) { // This is a kludge
      d_push(task, i);
      task->pcp++;
    } else {
      cb_program_exec_word(progp, task);
    }
  }
}

/*
We may be running different programs in the same task.
Dictionary entries have their own program
*/
static void ef_do(program_p progp, ftask_p task) {
  p_code_p pcp = *task->pcp;
  long l1 = d_pop(task);
  long l2 = d_pop(task);

  for (int i = 0; i < (l2 - l1); i++) {
    // printf("\nLoopcnt: %d pcnt:%d", i, pcnt);
    // program_dump(progp, pcnt);
    run_block(progp, task, i + l1);
  }
}

/*
We may be running different programs in the same task.
Dictionary entries have their own program
*/
static void ef_do_new(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  task->loop_lower = d_pop(task);
  task->loop_upper = d_pop(task);
  task->pcp++;
}

/**
jump to code at DO
*/
static inline void ef_loop_end(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  if (task->loop_lower++ >= task->loop_upper) {
    task->pcp++;
    return;
  }
  // printf("LOOP BACK TO %ld\n", pcp->val.l);
  task->pcp = prog->pcp_array + pcp->val.l;
}

/*
Check ToS, if TRUE, execute the code that follows.
If FALSE, jump to code after ELSE or THEN (stored in pcp->val.l)
*/
static inline void ef_if(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  if (d_pop(task) == 0) {
    // printf("IF CODE TRUE, FALL THROUGH\n");
    task->pcp++;
  } else {
    // printf("IF CODE FALSE, SKIP TO PAST ELSE OR THEN(%p)\n",
    // pcp->val.jump_to);
    task->pcp = prog->pcp_array + pcp->val.l + 1;
  }
}

static inline void ef_else(program_p progp, ftask_p task) {
  p_code_p pcp = *task->pcp;
  // printf("ELSE CAUGHT IT, SKIP TO THEN\n");
  task->pcp = progp->pcp_array + pcp->val.l;
}

static inline void ef_then(program_p progp, ftask_p task) {
  // printf("THEN CAUGHT IT, FALLING THROUG\n");
  task->pcp++;
}

static  void ef_exit(program_p prog, ftask_p task) {
  task->pcp = prog->pcp_array + prog->npcp_array ;
}

static inline void ef_last_code(program_p prog, ftask_p task) {
  printf("*** STOP RUN**\n");
  task->pcp++;
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
  PCODE_EXIT=11,
  PCODE_LAST*/
static cbp_exec_func farray[] = {
    ef_error, ef_primitive, cb_ef_number, ef_variable, ef_dict_entry,
    ef_if,    ef_do_new,    ef_loop_end,  ef_i_cb,     ef_else,
    ef_then,  ef_exit,      ef_last_code};

static void cb_program_exec_word(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  char buf[128];
  sprintf(buf, "%p %s%d", task->pcp, prog->name, pcp->type);
  logg("EXEC", buf);

  if (STEP) {
    d_stack_dump(task);
    program_dump(prog, task);
    printf("\n...");
    if (getchar() == 'x') {
      STEP = 0;
    }
  }
  farray[pcp->type](prog, task);
}

/**
Loop through the program, calling func for each p-code
*/
static int program_exec_loop(program_p prog, cbp_exec_func func,
                             ftask_p task) {
  task->pcp = prog->pcp_array;
  while (task->pcp < prog->pcp_array + prog->npcp_array) {
    func(prog, task);
  }
  return 0;
}

void run_prog(ftask_p task, program_p prog) {
  if (STEP) {
    printf("\nRUNNING:\n");
    program_dump(prog, task);
  }
  program_exec_loop(prog, cb_program_exec_word, task);
}

void run_task(ftask_p task) { run_prog(task, task->program); }
