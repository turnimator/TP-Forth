/*
 * smtoks.c
 *
 *  Created on: Sep 27, 2024
 *      Author: Jan Atle Ramsli
 *
 */
#include "smtok.h"

#include "builtins.h"
#include "dictionary.h"
#include "task.h"
#include "variable.h"
#include <ctype.h>
#include <string.h>

int isNumber(char *tok) {
  if (!isdigit(*tok) && *tok != '-') {
    return 0;
  }
  for (char *p = tok + 1; *p; p++) {
    if (!isdigit(*p)) {
      return 0;
    }
  }
  return 1;
}
/**
Create an empty p-code to be filled in
*/
static smtok_p smtok_create_empty(char *name) {
  smtok_p rv = malloc(sizeof(smtok_t));
  memset(rv, 0, sizeof(smtok_t));
  rv->name = malloc(strlen(name));
  strcpy(rv->name, name);
  return rv;
}

void smtok_delete(smtok_p pc) {
  free(pc->name);
  pc->name = 0;
  free(pc);
}

static smtok_p smtok_create_long(long l) {
  smtok_p rv = smtok_create_empty("NUM");
  rv->jtidx = PCODE_NUMBER;
  rv->val.l = l;
  return rv;
}

static smtok_p smtok_create_primitive(idx_builtin_p ip) {
  smtok_p rv = smtok_create_empty(ip->name);
  rv->val.pp = DB_builtins[ip->op];
  rv->jtidx = PCODE_BUILTIN;
  return rv;
}

static smtok_p smtok_create_dict_entry(dict_entry_p dep) {
  smtok_p rv = smtok_create_empty(dep->name);
  rv->jtidx = PCODE_DICT_ENTRY;
  rv->val.prog = dep->prog;
  return rv;
}

/**
 */
smtok_p smtok_ct_create(jumptable_idx_t pctyp, char *name) {
  smtok_p rv = smtok_create_empty(name);
  rv->jtidx = pctyp;
  return rv;
}

/// BUG? Check this. Variable lookup should only happen at compile time.
smtok_p smtok_create_variable(int idx) {
  var_p v = variable_get(idx);
  smtok_p rv = smtok_create_empty(v->name);
  rv->jtidx = PCODE_VARIABLE;
  rv->val.var_idx = v->vt_idx;
  return rv;
}

smtok_p smtok_create_I() {
  smtok_p rv = smtok_create_empty("IDX");
  rv->jtidx = PCODE_I;
  return rv;
}

smtok_p smtok_defer_create() {
  smtok_p rv = smtok_create_empty("DEFER");
  rv->jtidx = PCODE_DEFER;
  return rv;
}

smtok_p smtok_exec_create() {
  smtok_p rv = smtok_create_empty("EXEC");
  rv->jtidx = PCODE_EXEC;
  return rv;
}

smtok_p smtok_spawn_create() {
  smtok_p rv = smtok_create_empty("SPAWN");
  rv->jtidx = PCODE_SPAWN;
  return rv;
}

smtok_p smtok_string_create(char *s) {
  smtok_p rv = smtok_create_empty("STR");
  rv->jtidx = PCODE_STRING;
  rv->val.s = malloc(strlen(s));
  strcpy(rv->val.s, s + 1);
  rv->val.s[strlen(rv->val.s) - 1] = 0;
  return rv;
}

/**
Parse a word, creating a p-code
*/
smtok_p smtok_compile_word(char *src) {
  if (strcmp(src, "I") == 0) {
    return smtok_create_I();
  }
  if (isNumber(src)) {
    return smtok_create_long(atol(src));
  }
  if (*src == '\"') {
    return (smtok_string_create(src));
  }
  if (strcmp(src, "\'") == 0) {
    return smtok_defer_create();
  }
  if (strcmp(src, "EXEC") == 0) {
    return smtok_exec_create();
  }
  if (strcmp(src, "SPAWN") == 0) {
    return smtok_spawn_create();
  }

  int idx = variable_lookup(src);
  {
    if (idx != -1) {
      return smtok_create_variable(idx);
    }
  }

  idx_builtin_p ip = builtin_lookup(src);
  if (ip) {
    return smtok_create_primitive(ip);
  }

  dict_entry_p dep = dict_lookup(0, src);
  if (dep) {
    return smtok_create_dict_entry(dep);
  }
  printf("\n^^^^ %s NOT FOUND\n", src);
  return 0;
}
