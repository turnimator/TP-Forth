/*
 * p_codes.c
 *
 *  Created on: Sep 27, 2024
 *      Author: Jan Atle Ramsli
 *
 */
#include "p_code.h"
#include "builtins.h"
#include "dictionary.h"
#include "task.h"
#include "variable.h"
#include <ctype.h>
#include <string.h>

int isNumber(char *tok) {
  for (char *p = tok; *p; p++) {
    if (!isdigit(*p)) {
      return 0;
    }
  }
  return 1;
}
/**
Create an empty p-code to be filled in
*/
static p_code_p p_code_create_empty(char *name) {
  p_code_p rv = malloc(sizeof(p_code_t));
  memset(rv, 0, sizeof(p_code_t));
  rv->name = malloc(strlen(name));
  strcpy(rv->name, name);
  return rv;
}

void p_code_delete(p_code_p pc) {
  free(pc->name);
  pc->name = 0;
  free(pc);
}

static p_code_p p_code_create_long(long l) {
  p_code_p rv = p_code_create_empty("NUM");
  rv->type = PCODE_NUMBER;
  rv->val.l = l;
  return rv;
}

static p_code_p p_code_create_primitive(idx_builtin_p ip) {
  p_code_p rv = p_code_create_empty(ip->name);
  rv->val.pp = DB_builtins[ip->op];
  rv->type = PCODE_BUILTIN;
  return rv;
}

static p_code_p p_code_create_dict_entry(dict_entry_p dep) {
  p_code_p rv = p_code_create_empty(dep->name);
  rv->type = PCODE_DICT_ENTRY;
  rv->val.prog = dep->prog;
  return rv;
}

/**
 */
p_code_p p_code_ct_create(p_code_type_t pctyp, char *name) {
  p_code_p rv = p_code_create_empty(name);
  rv->type = pctyp;
  return rv;
}

/// BUG? Check this. Variable lookup should only happen at compile time.
p_code_p p_code_create_variable(int idx) {
  var_p v = variable_get(idx);
  p_code_p rv = p_code_create_empty(v->name);
  rv->type = PCODE_VARIABLE;
  rv->val.var_idx = v->vt_idx;
  return rv;
}

p_code_p p_code_create_I() {
  p_code_p rv = p_code_create_empty("IDX");
  rv->type = PCODE_I;
  return rv;
}

p_code_p p_code_defer_create(){
	p_code_p rv = p_code_create_empty("DEFER");
  rv->type = PCODE_DEFER;
  return rv;
}

p_code_p p_code_exec_create(){
	p_code_p rv = p_code_create_empty("EXEC");
  rv->type = PCODE_EXEC;
  return rv;
}

p_code_p p_code_spawn_create(){
	p_code_p rv = p_code_create_empty("SPAWN");
  rv->type = PCODE_SPAWN;
  return rv;
}

p_code_p p_code_string_create(char*s){
	p_code_p rv = p_code_create_empty("STR");
	rv->type=PCODE_STRING;
	rv->val.s = malloc(strlen(s));
	strcpy(rv->val.s, s+1);
	rv->val.s[strlen(rv->val.s)-1] = 0;
	return rv;
}

/**
Parse a word, creating a p-code
*/
p_code_p p_code_compile_word(char *src) {
  if (strcmp(src, "I") == 0) {
    return p_code_create_I();
  }
  if (isNumber(src)) {
    return p_code_create_long(atol(src));
  }
  if (*src=='\"'){
	return (p_code_string_create(src));
  }
  if (strcmp(src, "DEFER")==0){
	return p_code_defer_create();
  }
  if (strcmp(src, "EXEC")==0){
	return p_code_exec_create();
  }
  if (strcmp(src, "SPAWN")==0){
	return p_code_spawn_create();
  }

  int idx = variable_lookup(src);
  {
    if (idx != -1) {
      return p_code_create_variable(idx);
    }
  }
  idx_builtin_p ip = builtin_lookup(src);
  if (ip) {
    return p_code_create_primitive(ip);
  }

  dict_entry_p dep = dict_lookup(0, src);
  if (dep) {
    return p_code_create_dict_entry(dep);
  }
  printf("\n^^^^ %s NOT FOUND\n", src);
  return 0;
}
