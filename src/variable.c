/*
 * variable.c
 *
 *  Created on: Oct 4, 2024
 *      Author: Jan Atle Ramsli
 *
 */
#include "variable.h"
#include "logg.h"
#include "program.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h> 

var_p *vartable = 0;
int n_vartable = 0;

static void vt_add(var_p vp) {
  n_vartable++;
  if (vartable) {
    vartable = realloc(vartable, sizeof(var_p *) * n_vartable);
  } else {
    vartable = malloc(sizeof(var_p *) * n_vartable);
  }
  vp->vt_idx = n_vartable - 1;
  vartable[n_vartable - 1] = vp;
}

char *vartype_string(var_p v) {
  static char buf[32];
  buf[0] = 0;
  switch (v->t) {
  case VTYP_ADDR:
    sprintf(buf, "ADDR %p", v->val.addr);
    break;
  case VTYP_LONG:
    sprintf(buf, "LONG %ld", v->val.l);
    break;
  case VTYP_DOES:
    sprintf(buf, "PROG %s", v->does->name);
    break;
  }
  return buf;
}

int vt_dump_cb(var_p v, char *name) {
  printf("VAR:%s=%s\n", v->name, vartype_string(v));
  return 0;
}

int vt_lookup_cb(var_p v, char *name) {
  return (strcmp(name, v->name) == 0 ? 1 : 0);
}

int vt_loop(int (*vt_func)(var_p, char *), char *p) {
  for (int i = 0; i < n_vartable; i++) {
    if (vt_func(vartable[i], p)) {
      return i;
    }
  }
  return -1;
}

void variable_dump() {
  printf("----------|VARIABLES|-----------\n");
  vt_loop(vt_dump_cb, 0);
  printf("---------------------------------\n");
}

var_p variable_get(int idx) 
{ 
	if(idx<n_vartable){
	return vartable[idx];
	} else {
		printf("ERROR: VARIABLE INDEX %d OUT OF RANGE", idx);
	}
	return 0;
}

int variable_lookup(char *name) 
{ 
	return vt_loop(vt_lookup_cb, name); 
}

static var_p var_create(char *name) {
  var_p rv = malloc(sizeof(var_t));
  rv->name = malloc(strlen(name));
  strcpy(rv->name, name);
  rv->val.l = 0;
  return rv;
}

void var_delete(var_p var) {
  free(var->name);
  free(var);
}

void var_store_long(var_p v, long l) {
  v->val.l = l;
  v->t = VTYP_LONG;
}

void var_store_addr(var_p v, char *addr) {
  v->val.addr = addr;
  v->t = VTYP_ADDR;
}

void variable_does(var_p v, program_p prog) {
  v->does = prog;
  v->t = VTYP_DOES;
}

var_p variable_add(char *name) {
  logg("NAME", "VARTABLE BEFORE:");
  int idx = variable_lookup(name);
  if (idx != -1){
	printf("VARIABLE %s redefined", name);
	return vartable[idx];
  }
  var_p rv = var_create(name);
  vt_add(rv);
  #ifdef DEBUG
  variable_dump();
  #endif
  return rv;
}
