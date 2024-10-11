/*
 * program.c
 *
 *  Created on: Sep 29, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "program.h"
#include "logg.h"
#include "p_code.h"
#include "variable.h"
#include <string.h>

/**
Loop through the program, calling func for each p-code
*/
void program_loop(program_p progp, void (*func)(program_p, p_code_p, ftask_p),
                  ftask_p task) {
  for (int i = 0; i < progp->npcp_array; i++) {
    func(progp, progp->pcp_array[i], task);
  }
}

int offs(program_p prog, p_code_p *p) 
{
  int program_offset;
  for (program_offset = 0; program_offset < prog->npcp_array;
       program_offset++) {
    if (prog->pcp_array + program_offset == p) {
      return program_offset;
    }
  }
  return program_offset;
}

void program_dump_cb(program_p prog, p_code_p pcp, ftask_p task) {
  var_p v;
  int ihere, ijump;
  if (pcp == *task->pcp) {
    printf(" ==>");
  } 
  ihere = offs(prog, task->pcp);
  ijump = pcp->val.l;

  switch (pcp->type) {
  case PCODE_NUMBER:
    printf("NUM:%ld ", pcp->val.l);
    return;
  case PCODE_BUILTIN:
    printf("BIN:%s ", pcp->name);
    return;
  case PCODE_VARIABLE:
    v = variable_get(pcp->val.var_idx);
    printf("VAR:(%s=%s) ", pcp->name, vartype_string(v));
    return;
  case PCODE_DICT_ENTRY:
    printf("DEF:%s ", pcp->name);
    program_dump(pcp->val.prog, task);
    return;
  case PCODE_IF:
    printf("IF:%d ", ijump);
    return;
  case PCODE_ELSE:
    printf("ELSE:%d ", ijump);
    return;
  case PCODE_THEN:
    printf("THEN:%d ", ijump);
    return;
  case PCODE_LOOP_DO:
    printf("DO:%d ", ijump);
    return;
  case PCODE_LOOP_END:
    printf("LOOP:%d ", ijump);
    return;
  case PCODE_I:
    printf("I:%s ", pcp->name);
    return;
    break;
    case PCODE_LAST:
    printf("END ");
  default:
    printf("?");
    return;
    break;
  }
  printf("\nShould never arrive here\n");
}

void program_dump(program_p progp, ftask_p task) {
  printf("\n----------------| %s |-----------------\n", progp->name);
  program_loop(progp, program_dump_cb, task);
  printf("\n----------------------------------------\n\n");
}

static void program_delete_cb(program_p prog, p_code_p pcp, ftask_p task) {
  p_code_delete(pcp);
}

void program_delete(program_p prog) {
  program_loop(prog, program_delete_cb, 0);
  prog->npcp_array = 0;
  free(prog->pcp_array);
  prog->pcp_array = 0;
  free(prog->name);
  prog->name = 0;
  free(prog);
}

program_p program_create(char *name) {
  program_p prog = malloc(sizeof(program_p));
  prog->pcp_array = malloc(sizeof(p_code_p *));
  prog->name = malloc(strlen(name));
  strcpy(prog->name, name);
  prog->npcp_array = 0;
  return prog;
}

void program_add_p_code(program_p prog, p_code_p pcp) {
  logg(prog->name, pcp->name);

  prog->npcp_array++;
  prog->pcp_array =
      realloc(prog->pcp_array, sizeof(p_code_p *) * prog->npcp_array);
  prog->pcp_array[prog->npcp_array - 1] = pcp;
}

p_code_p *program_last(program_p prog) {
  return prog->pcp_array + prog->npcp_array - 1;
}

void program_add(program_p prog, char *src) {
  p_code_p pc = p_code_parse_word(src);
  if (!pc) {
    printf("\nprogram_add(%s) failed \n", src);
    return;
  }
  program_add_p_code(prog, pc);
}
