/*
 * variable.h
 *
 *  Created on: Oct 4, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "task.h"
enum variable_type { VTYP_LONG, VTYP_ADDR, VTYP_STR, VTYP_ARRAY, VTYP_DOES };

typedef union addrtype {
  char *cp;
  long *lp;
} varaddr_t;

typedef struct varval {
  varaddr_t addr;
  long l; // Scalar or element number in case of array
  int sz; // Element size in case of array
} varval_t, *varval_p;

typedef struct variable {
  varval_t val;
  char *name;
  int vt_idx;
  enum variable_type t;
  program_p does;
} var_t, *var_p;

var_p variable_add(char *name);
var_p variable_get(int idx);
int variable_lookup(char *);
char *vartype_string(var_p v);
void var_store_long(var_p v, long l);
void var_store_addr(var_p v, char *addr);
void vartable_dump();

#endif /* VARIABLE_H_ */
