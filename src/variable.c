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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

var_p *vartable = 0;
int n_vartable = 0;

static void vartable_add(var_p vp) {
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
    sprintf(buf, "ADDR %p", v->val.addr.cp);
    break;
  case VTYP_LONG:
    sprintf(buf, "LONG %ld", v->val.l);
    break;
  case VTYP_DOES:
    sprintf(buf, "PROG %s", v->does->name);
    break;
  case VTYP_ARRAY:
    sprintf(buf, "ARRAY %s[%ld]", v->name, v->val.l);
    break;
  case VTYP_STR:
    sprintf(buf, "STR %s %s", v->name, v->val.addr.cp);
    break;
  }
  return buf;
}

int vartable_dump_cb(var_p v, char *name) {
  printf("VAR:%s=%s\n", v->name, vartype_string(v));
  return 0;
}

int vartable_lookup_cb(var_p v, char *name) {
  return (strcmp(name, v->name) == 0 ? 1 : 0);
}

int vartable_forEach(int (*vt_func)(var_p, char *), char *p) {
  for (int i = 0; i < n_vartable; i++) {
    if (vt_func(vartable[i], p)) {
      return i;
    }
  }
  return -1;
}

void vartable_dump() {
  printf("----------|VARIABLES|-----------\n");
  vartable_forEach(vartable_dump_cb, 0);
  printf("---------------------------------\n");
}

var_p variable_get(int idx) {
  if (idx < n_vartable) {
    return vartable[idx];
  } else {
    printf("ERROR: VARIABLE INDEX %d OUT OF RANGE", idx);
  }
  return 0;
}

int variable_lookup(char *name) {
  return vartable_forEach(vartable_lookup_cb, name);
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
  v->val.addr.cp = addr;
  v->t = VTYP_ADDR;
}

void variable_does(var_p v, program_p prog) {
  v->does = prog;
  v->t = VTYP_DOES;
}


var_p variable_add(char *name) {
  logg("NAME", "VARTABLE BEFORE:");
  int idx = variable_lookup(name);
  if (idx != -1) {
    printf("VARIABLE %s redefined", name);
    return vartable[idx];
  }
  var_p rv = var_create(name);
  vartable_add(rv);
#ifdef DEBUG
  variable_dump();
#endif
  return rv;
}
