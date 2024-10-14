/*
 * fnf_p_machine.c
 *
 *  Created on: Sep 21, 2024
 *      Author: Jan Atle Ramsli
 *
 */
// #define DEBUG

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
  logg(prog->name, "P-CODE ERROR");
  task->pcp = prog->pcp_array + prog->npcp_array;
}

inline p_code_p *p_code_here(ftask_p task) { return task->pcp; }

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
  logg(prog->name, "");
  p_code_p pcp = *task->pcp;
  dict_entry_p sub = 0;
  sub = dict_lookup(0, pcp->name);
  if (!sub) {
    printf("\nEXEC: %s dict_lookup failed!", pcp->name);
    task->pcp++;
  }
  r_push(task, task->pcp);
  // program_dump(prog,task);

  run_prog(task, sub->prog);
  task->pcp = r_pop(task);
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
  d_push(task, ll_tos(task));
  task->pcp++;
}

/*
We may be running different programs in the same task.
Dictionary entries have their own program
 */
static void ef_do(program_p prog, ftask_p task) {
  ll_push(task, d_pop(task));
  lu_push(task, d_pop(task));
#ifdef DEBUG
  printf("DO FROM %d TO %d \n", ll_tos(task), lu_tos(task));
#endif
  task->pcp++;
  r_push(task, task->pcp); // point to next code
}

long offs(program_p prog, p_code_p *pc) { return pc - prog->pcp_array; }

/**
jump to code at DO
Using macros for speed here
 */
static inline void ef_loop_end(program_p prog, ftask_p task) {
  logg("LOOP", "");
  if (ll_tos(task)++ >= lu_tos(task)) {
#ifdef DEBUG
    printf("***LOOP BREAK TO %ld ***\n", offs(prog, task->pcp));
#endif
    ll_tos(task) = 0;
    lu_tos(task) = 0;
    ll_pop(task); // Get our old values back in case of nested loops
    lu_pop(task);
    task->pcp++;
    r_pop(task);
    return;
  }
#ifdef DEBUG
  printf("LOOP BACK TO %ld\n", offs(prog, r_tos(task)));
#endif
  task->pcp = r_tos(task);
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

static inline void ef_else(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  // printf("ELSE CAUGHT IT, SKIP TO THEN\n");
  task->pcp = prog->pcp_array + pcp->val.l;
}

static inline void ef_then(program_p prog, ftask_p task) {
  // printf("THEN CAUGHT IT, FALLING THROUG\n");
  task->pcp++;
}

static void ef_exit(program_p prog, ftask_p task) {
  logg("Program before", prog->name);
  task->pcp = prog->pcp_array + prog->npcp_array;
  logg("Program after", prog->name);
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
    ef_if,    ef_do,        ef_loop_end,  ef_i_cb,     ef_else,
    ef_then,  ef_exit,      ef_last_code};

char *tstr(enum p_code_type t) {
  static char *sarray[] = {"PCODE_ERROR", "PCODE_BUILTIN", "PCODE_NUMBER",
                           "VARIABLE",    "DICT_ENTRY",    "IF",
                           "DO",          "LOOP",          "I",
                           "ELSE",        "THEN",          "EXIT",
                           "PCODE_LAST"};
  return sarray[t];
}

static void cb_program_exec_word(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  char buf[128];
  sprintf(buf, "%s:%s(%s)", prog->name, tstr(pcp->type), pcp->name);
  logg("EXEC->", buf);

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
TODO BREAK ON ;
 */
static int program_exec_loop(program_p prog, cbp_exec_func func, ftask_p task) {
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
