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
  int var_idx; // Index into variable table
  program_p prog;
  p_code_p* jump_to;
} p_val_t, *p_val_p;

typedef enum p_code_type {
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
  PCODE_LAST

} p_code_type_t;

typedef struct p_code {
  char *name;
  p_code_type_t type;
  p_val_t val;
} p_code_t, *p_code_p;

int isNumber(char *tok);

p_code_p p_code_parse_word(char *);
p_code_p p_code_ct_create(p_code_type_t);
p_code_p p_code_create_variable(int);
void p_code_delete(p_code_p pc);

#endif /* P_CODE_H_ */
