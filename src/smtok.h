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
  SMTOK_ERROR = 0,
  SMTOK_BUILTIN = 1,
  SMTOK_NUMBER = 2,
  SMTOK_VARIABLE = 3,
  SMTOK_DICT_ENTRY = 4,
  SMTOK_IF = 5,
  SMTOK_LOOP_DO = 6,
  SMTOK_LOOP_END = 7,
  SMTOK_I = 8,
  SMTOK_ELSE = 9,
  SMTOK_THEN =10,
  SMTOK_EXIT = 11,
  SMTOK_DEFER = 12,
  SMTOK_EXEC = 13,
  SMTOK_SPAWN =14,
  SMTOK_STRING = 15,
  SMTOK_LOOP_BEGIN = 16,
  SMTOK_LOOP_AGAIN = 17,
  SMTOK_LOOP_UNTIL = 18,
  SMTOK_LAST

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
