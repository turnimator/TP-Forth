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

void r_push(program_p prog, program_p newprog) 
{
  prog->r_stack[prog->r_top] = newprog;
  prog->r_top++;
}

program_p r_pop(program_p prog) 
{
  if (prog->r_top < 0) {
    printf("\n****Program Return Stack Underflow***");
    return 0;
  }
  prog->r_top--;
  return prog->r_stack[prog->r_top];
}


static void ef_error(program_p prog, ftask_p task) {
  logg(prog->name, "P-CODE ERROR");
  prog->pcp = prog->pcp_array + prog->npcp_array;
}

static inline void cb_ef_number(program_p prog, ftask_p task) {
  p_code_p pcp = *prog->pcp;
  d_push(task, pcp->val.l);
  prog->pcp++;
}

static inline void ef_primitive(program_p prog, ftask_p task) {
  p_code_p pcp = *prog->pcp;
  DB_builtins[pcp->val.pp->op]->code(task);
  prog->pcp++;
}

static inline void ef_variable(program_p prog, ftask_p task) {
  p_code_p pcp = *prog->pcp;
  v_push(task, pcp->val.var_idx);
  prog->pcp++;
}

/**
Look up the dictionary entry.
If found, run its program.
*/
static inline void ef_dict_entry(program_p prog, ftask_p task) {
  logg(prog->name, "");
  p_code_p pcp = *prog->pcp;
  dict_entry_p sub = 0;
  sub = dict_lookup(0, pcp->name);
  if (!sub) {
    printf("\nEXEC: %s dict_lookup failed!", pcp->name);
    prog->pcp++;
  }
  r_push(prog, sub->prog);
  prog->pcp = sub->prog->pcp_array;
  // program_dump(prog,task);
  run_prog(task, sub->prog);
  prog = r_pop(prog);
  prog->pcp++;
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
  prog->pcp++;
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

  prog->pcp++;
}

/**
jump to code at DO
Using macros for speed here
*/
static inline void ef_loop_end(program_p prog, ftask_p task) {
  logg("LOOP", "");
  p_code_p pcp = *prog->pcp;
  if (ll_tos(task)++ >= lu_tos(task)) {
#ifdef DEBUG
    printf("***LOOP BREAK TO %ld ***\n", prog->pcp - prog->pcp_array);
#endif
    prog->pcp++;
    ll_tos(task) = 0;
    lu_tos(task) = 0;
    ll_pop(task); // Get our old values back in case of nested loops
    lu_pop(task);
    return;
  }
#ifdef DEBUG
  printf("LOOP BACK TO %ld\n", pcp->val.l);
#endif
  prog->pcp = prog->pcp_array + pcp->val.l;
}

/*
Check ToS, if TRUE, execute the code that follows.
If FALSE, jump to code after ELSE or THEN (stored in pcp->val.l)
*/
static inline void ef_if(program_p prog, ftask_p task) {
  p_code_p pcp = *prog->pcp;
  if (d_pop(task) == 0) {
    // printf("IF CODE TRUE, FALL THROUGH\n");
    prog->pcp++;
  } else {
    // printf("IF CODE FALSE, SKIP TO PAST ELSE OR THEN(%p)\n",
    // pcp->val.jump_to);
    prog->pcp = prog->pcp_array + pcp->val.l + 1;
  }
}

static inline void ef_else(program_p prog, ftask_p task) {
  p_code_p pcp = *prog->pcp;
  // printf("ELSE CAUGHT IT, SKIP TO THEN\n");
  prog->pcp = prog->pcp_array + pcp->val.l;
}

static inline void ef_then(program_p prog, ftask_p task) {
  // printf("THEN CAUGHT IT, FALLING THROUG\n");
  prog->pcp++;
}

static void ef_exit(program_p prog, ftask_p task) {
  logg("Program before", prog->name);
  prog->pcp = prog->pcp_array + prog->npcp_array;
  prog = r_pop(prog);
  logg("Program after", prog->name);
}

static inline void ef_last_code(program_p prog, ftask_p task) {
  printf("*** STOP RUN**\n");
  prog->pcp++;
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
  static char *sarray[] = {"PCODE_ERROR",    "PCODE_BUILTIN",    "PCODE_NUMBER",
                           "PCODE_VARIABLE", "PCODE_DICT_ENTRY", "PCODE_IF",
                           "PCODE_LOOP_DO",  "PCODE_LOOP_END",   "PCODE_I",
                           "PCODE_ELSE",     "PCODE_THEN",       "PCODE_EXIT",
                           "PCODE_LAST"};
  return sarray[t];
}
static void cb_program_exec_word(program_p prog, ftask_p task) {
  p_code_p pcp = *prog->pcp;
  char buf[128];
  sprintf(buf, "%s:%s", prog->name, tstr(pcp->type));
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
*/
static int program_exec_loop(program_p prog, cbp_exec_func func, ftask_p task) {
  prog->pcp = prog->pcp_array;
  while (prog->pcp < prog->pcp_array + prog->npcp_array) {
    if (prog->pcp) {
      func(prog, task);
    }
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
