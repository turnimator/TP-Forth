/*
 * fnf_p_machine.c
 *
 *  Created on: Sep 21, 2024
 *      Author: Jan Atle Ramsli
 *
 */
// #define DEBUG
#include "runtime.h"
#include <pthread.h>

#include "builtins.h"
#include "logg.h"
#include "p_code.h"
#include "program.h"
#include "task.h"

extern int STEP;

typedef void (*cbp_exec_func)(program_p, ftask_p);
static cbp_exec_func p_code_jump_table[];

long offs(program_p prog, p_code_p *pc) { return pc - prog->pcp_array; }

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
  d_push(task, pcp->val.var_idx);
  task->pcp++;
}

static inline void ef_dict_entry(program_p prog, ftask_p task) {
  logg(prog->name, "");
  p_code_p pcp = *task->pcp;
  // r_push(task, task->pcp + 1);
  r_push(task, task->pcp);
  // program_dump(prog,task);
  prog_push(task, task->program);
  run_prog(task, pcp->val.prog);
  task->program = prog_pop(task);
  task->pcp = r_pop(task) + 1;
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
  ld_push(task,(lu_tos(task)-ll_tos(task)>0)?1:-1);
  
#ifdef DEBUG
  printf("DO FROM %d TO %d \n", ll_tos(task), lu_tos(task));
#endif
  task->pcp++;
  r_push(task, task->pcp); // point to next code after DO
}

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
    ld_pop(task);
    r_pop(task); // Pop off the return value that was pushed in ef_do
    task->pcp++;
    // prog_pop(task);
    return;
  }
#ifdef DEBUG
  printf("LOOP BACK TO %ld\n", offs(prog, r_tos(task)));
#endif
  task->pcp =
      r_tos(task); // LOOP back to p_code pushed in the last line of ef_do
}

/*
Check ToS, if TRUE, execute the code that follows.
If FALSE, jump to code after ELSE or THEN (stored in pcp->val.l)
 */
static inline void ef_if(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  if (d_pop(task) == F_TRUE) {
    // printf("IF CODE TRUE, FALL THROUGH\n");
    task->pcp++;
  } else {
    // printf("IF CODE FALSE, SKIP TO PAST ELSE OR THEN(%p)\n",
    // pcp->val.jump_to);
    task->pcp += pcp->val.l;
  }
}

static inline void ef_else(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  // printf("ELSE CAUGHT IT, SKIP TO THEN\n");
  task->pcp += pcp->val.l;
}

static inline void ef_then(program_p prog, ftask_p task) {
  // printf("THEN CAUGHT IT, FALLING THROUG\n");
  task->pcp++;
}

static void ef_exit(program_p prog, ftask_p task) {
  loggssn("Jump to end of", prog->name, prog->npcp_array);
  task->pcp = prog->pcp_array + prog->npcp_array;
}

static inline void ef_last_code(program_p prog, ftask_p task) {
  printf("*** STOP RUN**\n");
  task->pcp++;
}

// Place next P-Code on stack instead of executing
static inline void ef_defer(program_p prog, ftask_p task) {
  logg("DEFER", "");
  task->pcp++; // Point to the deferred word
  d_push(task, (long)task->pcp);
  task->pcp++; // Skip the deferred word
}

static inline void ef_exec(program_p prog, ftask_p task) {
  r_push(task, task->pcp);
  p_code_p *to_be_executed = (p_code_p *)d_pop(task);
  task->pcp = to_be_executed;
  cb_program_exec_word(prog, task);
  task->pcp = r_pop(task);
  task->pcp++;
}

static void *spawnfunc(void *v) {
  logg("TASK", "SPAWNED");
  run_task((ftask_p)v);
  return 0;
}

static pthread_t tid[128];
static int idx_tid = 0;

static inline void ef_spawn(program_p prog, ftask_p task) {
  logg("SPAWNING", "TASK");
  r_push(task, task->pcp);
  p_code_p *to_be_executed = (p_code_p *)d_pop(task);
  ftask_p new_task = ftask_create("tsk");
  new_task->program = program_create("tsk");
#ifdef DEBUG
  program_dump(new_task->program, new_task);
#endif
  idx_tid++;
  if (idx_tid > 127) {
    idx_tid = 0;
  }

  program_add_p_code(new_task->program, *to_be_executed);
  int err = pthread_create(&tid[idx_tid], NULL, spawnfunc, (void *)new_task);
  if (err) {
    perror("task creation failed");
  }
  task->pcp = r_pop(task);
  task->pcp++;
}

static inline void ef_string(program_p prog, ftask_p task) {
  p_code_p p = *task->pcp;
  d_push(task, (long)p->val.s);
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
  PCODE_EXIT = 11,
  PCODE_DEFER = 12,
  PCODE_EXEC = 13,
  PCODE_SPAWN =14,
  PCODE_STRING = 15,
  PCODE_LAST*/
static cbp_exec_func p_code_jump_table[] = {
    ef_error, ef_primitive, cb_ef_number, ef_variable, ef_dict_entry, ef_if,
    ef_do,    ef_loop_end,  ef_i_cb,      ef_else,     ef_then,       ef_exit,
    ef_defer, ef_exec,      ef_spawn,     ef_string,   ef_last_code};

char *tstr(jumptable_idx_t t) {
  static char *jts_sarray[] = {"PCODE_ERROR", "PCODE_BUILTIN", "PCODE_NUMBER",
                               "VARIABLE",    "DICT_ENTRY",    "IF",
                               "DO",          "LOOP",          "I",
                               "ELSE",        "THEN",          "EXIT",
                               "QUOT",        "EXEC",          "SPWN",
                               "STR",         "PCODE_LAST"};
  return jts_sarray[t];
}

static void cb_program_exec_word(program_p prog, ftask_p task) {
  p_code_p pcp = *task->pcp;
  char buf[128];
  sprintf(buf, "%s:%s(%s)=%ld", prog->name, tstr(pcp->jtidx), pcp->name,
          pcp->val.l);
  logg("EXEC->", buf);

  if (STEP) {
    d_stack_dump(task);
    program_dump(prog, task);
    d_stack_dump(task);
    printf("\n...");
    if (getchar() == 'x') {
      STEP = 0;
    }
  }
  p_code_jump_table[pcp->jtidx](prog, task);
}

/**
Loop through the program, calling func for each p-code
TODO BREAK ON ;
 */
static int program_exec_loop(program_p prog, cbp_exec_func func, ftask_p task) {
  task->pcp = prog->pcp_array;
  while (task->pcp < (prog->pcp_array + prog->npcp_array)) {
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
