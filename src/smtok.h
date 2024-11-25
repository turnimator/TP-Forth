/*
 * 
 *
 *  Created on: Sep 27, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef SMTOK_H_
#define SMTOK_H_

#include "builtins.h"
#include "variable.h"
typedef union smtok_val {
  builtin_p pp;
  long l;
  char* s;
  int var_idx; // Index into variable table
  program_p prog;
} smtok_val_t, *smtok_val_p;

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

typedef struct smtok {
  char *name;
  jumptable_idx_t jtidx;
  smtok_val_t val;
} smtok_t, *smtok_p;

int isNumber(char *tok);

smtok_p smtok_compile_word(char *);
smtok_p smtok_ct_create(jumptable_idx_t, char*name);
smtok_p smtok_create_variable(int);
void smtok_delete(smtok_p pc);

#endif /* SMTOK_H_ */
