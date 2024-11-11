/*
 * p_code.h
 *
 *  Created on: Sep 27, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef P_CODE_H_
#define P_CODE_H_

#include "builtins.h"
#include "variable.h"
typedef union p_code_val {
  builtin_p pp;
  long l;
  char* s;
  int var_idx; // Index into variable table
  program_p prog;
} p_val_t, *p_val_p;

typedef enum jump_table_idx {
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
  PCODE_LOOP_BEGIN = 16,
  PCODE_LOOP_AGAIN = 17,
  PCODE_LOOP_UNTIL = 18,
  PCODE_LAST

} jumptable_idx_t;

typedef struct p_code {
  char *name;
  jumptable_idx_t jtidx;
  p_val_t val;
} p_code_t, *p_code_p;

int isNumber(char *tok);

p_code_p p_code_compile_word(char *);
p_code_p p_code_ct_create(jumptable_idx_t, char*name);
p_code_p p_code_create_variable(int);
void p_code_delete(p_code_p pc);

#endif /* P_CODE_H_ */
