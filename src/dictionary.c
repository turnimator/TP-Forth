/*
 * fnf_dict.c
 *
 *  Created on: Sep 21, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

#include "builtins.h"
#include "p_code.h"
#include "program.h"

dict_p Default_dict;

void dict_init(void) { Default_dict = dict_create("DEFAULT"); }

dict_p dict_create(char *name) {
  dict_p rv = malloc(sizeof(dict_t));
  rv->name = malloc(strlen(name) + 1);
  rv->dep_array = malloc(sizeof(dict_entry_p *));
  strcpy(rv->name, name);
  return rv;
}

dict_entry_p dict_loop(dict_p dp, int (*callback)(dict_entry_p, void *),
                       void *parm) {
  if (!dp) {
    dp = Default_dict;
  }
  for (int i = 0; i < dp->ndep_array; i++) {
    if (callback(dp->dep_array[i], parm))
      return (dp->dep_array[i]);
  }
  return 0;
}

static inline int cb_lookup(dict_entry_p e, void *p) {
  return (strcmp(e->name, (char*)p) == 0) ? 1 : 0;
}

dict_entry_p dict_lookup(dict_p dp, char *key) {
  if (!dp) {
    dp = Default_dict;
  }
  return dict_loop(dp, cb_lookup, (void *)key);
}

static int cb_dict_dump(dict_entry_p dep, void *p) {
  idx_builtin_p ip;
  printf("%s(%s):", dep->name, dep->prog->name);
  for (int i = 0; i < dep->prog->npcp_array; i++) {

    p_code_p pcp = dep->prog->pcp_array[i];

    switch (pcp->type) {
    case PCODE_NUMBER:
      printf("NUM:%ld ", pcp->val.l);
      break;
    case PCODE_BUILTIN:
      printf("BLTIN:%s ", pcp->name);
      ip = builtin_lookup(pcp->name);
      break;
    case PCODE_VARIABLE:
      printf("VAR:%s %ld", pcp->name, pcp->val.l);
      break;
    case PCODE_DICT_ENTRY:
      printf("DEF:%s ", pcp->name);
      program_dump(pcp->val.prog);
      break;
      case PCODE_DO:
      printf("LOOP:%s ", pcp->name);
      break;
      case PCODE_LOOP:
      printf("LOOP:%s ", pcp->name);
      break;
      case PCODE_IF:
      printf("COND:%s ", pcp->name);
      break;
      case PCODE_ERROR:
      printf("ERR:%s ", pcp->name);
      break;
    default:
      printf("PCODE%d? ", pcp->type);
      break;
    }
  }
  printf("\n");
  return 0;
}
void dict_dump(dict_p dp) {
  if (!dp) {
    dp = Default_dict;
  }
  printf("\n---------------| DICT |--------------\n");
  dict_loop(dp, cb_dict_dump, 0);
  printf("\n------------------------------------\n");
}

void dict_add_entry(dict_p dp, dict_entry_p dep) {
  if (!dp) {
    dp = Default_dict;
  }
  
    dict_dump(dp);
  
  dp->ndep_array++;
  dp->dep_array[dp->ndep_array - 1] = dep;
  dp->dep_array = realloc(dp->dep_array, sizeof(dict_entry_p) * dp->ndep_array);
  
    dict_dump(dp);
  
}

dict_entry_p dict_entry_create(dict_p dp) {
  if (!dp) {
    dp = Default_dict;
  }

  dict_entry_p rv = malloc(sizeof(dict_entry_t));
  return rv;
}

void dict_entry_set_name(dict_entry_p dep, char *name) {
  
  dep->prog = program_create(name);
  dep->name = malloc(strlen(name));
  strcpy(dep->name, name);
}

void dict_entry_add_word(dict_entry_p dep, char *word) {
 
  program_add(dep->prog, word);
}
