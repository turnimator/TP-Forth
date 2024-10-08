/*
 * parser.c
 *
 *  Created on: Sep 22, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "dictionary.h"
#include "logg.h"
#include "p_code.h"
#include "program.h"
#include "task.h"
#include "variable.h"

//IF ELSE THEN  
// IF set FALSE JUMP address to 0
// ELSE set FALSE JUMP address to HERE+1
// THEN if FALSE JUMP address is 0, set FALSE JUMP address to HERE+1

p_code_p current_IF;          // Current conditional
p_code_p current_ELSE;

p_code_p current_DO; // Current DO

static dict_entry_p de = 0; // Current colon definition

int isForthChar(char c) {
  if (isalnum(c)) {
    return 1;
  }
  if (strchr("`~!@#$%^&*()_-=<>?/[{]}\\|\",", c)) {
    return 1;
  }
  return 0;
}

int isForthCommentStart(char *tok) {
  if (strcmp(tok, "(") == 0 || strcmp(tok, "\\") == 0) {
    return 1;
  }
  return 0;
}

int isForthCommentEnd(char *tok) {
  if (strcmp(tok, ")") == 0 || strcmp(tok, "\n") == 0) {
    return 1;
  }
  return 0;
}

/**
* IF TRUE, fall though. When encountering ELSE skip to THEN
* IF FALSE skip to ELSE+1 (thereby executing ELSE code) or THEN+1
*/
static void if_create(program_p prog) {
  ct_push(CT_IF, current_IF); // In case of a nested IF, tuck away the old
  current_IF = p_code_ct_create(PCODE_IF); // and enter the new
  current_IF->val.l = 0;
  program_add_p_code(prog, current_IF);
}

static void else_create(program_p prog) {
  current_IF->val.l = program_last(prog) + 1; // Make IF FALSE point here
  ct_push(CT_IF, current_IF);
  current_IF = p_code_ct_create(PCODE_ELSE); // 
	current_IF->val.l = 0;
  program_add_p_code(prog, current_IF);
}


/**
Look at the compile time stack.
If the current is ELSE, we must ct_pop and fixup before handling THEN.
**/
static void then_create(program_p prog, p_code_p cond) {
	if (current_IF->type==PCODE_ELSE){
		current_IF->val.l = program_last(prog) + 1;
		current_IF = ct_pop(CT_IF);
	}
  if (current_IF->val.l == 0){
	current_IF->val.l = program_last(prog) + 1;
  }
  p_code_p pcode = p_code_ct_create(PCODE_THEN); // 
  pcode->val.l = program_last(prog) + 1;
  program_add_p_code(prog, pcode);
  cond = ct_pop(CT_IF);
}

/**
	runtime.c:
	DO: pop the two loop variables l1, l2.
	execute code between DO and LOOP l2-l1 times.
	while l1 < l2, fall through from here
	when l1=l2, jump to LOOP + 1 (contained on DO P-CODE)
	
	LOOP: jump to DO + 1 (contained in LOOP P-CODE)
	
**/
static void do_create(program_p prog) {
  ct_push(CT_DO, current_DO); // Must be popped on LOOP
  current_DO = p_code_ct_create(PCODE_LOOP_DO);
  current_DO->val.l = program_last(prog);
  program_add_p_code(prog, current_DO);
}

static void loop_create(program_p prog) {
  logg(prog->name, "ENTER");
  p_code_p loop_code;
  loop_code = p_code_ct_create(PCODE_LOOP_END);
  loop_code->val.l = current_DO->val.l+1; // Know wehere we must LOOP back to
  current_DO->val.l = program_last(prog) + 2; // Show DO where to skip LOOP
  program_add_p_code(prog, loop_code);
  current_DO = ct_pop(CT_DO);
}

static int parse_variable(program_p prog, char *name) {
  variable_add(name);
  return EXPECTING_ANY;
}

/**
ALL ACTIONS MUST BE PERFORMED HERE
*/
parser_state_t parse_word(program_p prog, char *tok) 
{
  logg("WORD", tok);
  if (strcmp(tok, ":") == 0) {
    de = dict_entry_create(0); // Action performed here for simplicity
    return COLON_EXPECTING_NAME;
  }
  if (strcmp(tok, "VARIABLE") == 0) {
    return VARIABLE_EXPECTING_NAME;
  }

  if (strcmp(tok, "IF") == 0) {
    if_create(prog);
    return IF_EXPECTING_ELSE_OR_THEN;
  }
  if (strcmp(tok, "THEN") == 0) {
    then_create(prog, current_IF); // and get back the old one
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "DO") == 0) {
    do_create(prog);
    return DO_EXPECTING_LOOP;
  }
  if (strcmp(tok, "LOOP") == 0) {
    loop_create(prog);
    return EXPECTING_ANY;
  }
  if (isForthCommentStart(tok)) {
    return PS_COMMENT;
  }
  if (strcmp(tok, ";") == 0) {
    logg(tok, "UNEXPECTED - NO DEFINITION");
    return EXPECTING_ANY;
  }
	if (strcmp(tok, "ELSE") == 0) {
    logg(tok, "UNEXPECTED - NO IF");
    return EXPECTING_ANY;
  }
	if (strcmp(tok, "THEN") == 0) {
    logg(tok, "UNEXPECTED - NO IF");
    return EXPECTING_ANY;
  }

  program_add(prog, tok);
  
  return EXPECTING_ANY;
}

parser_state_t parse_comment(char *tok) {
  logg("_", tok);
  if (isForthCommentEnd(tok)) {
    return EXPECTING_ANY;
  }
  return PS_COMMENT;
}

/**
The action is performed by the caller
*/
parser_state_t parse_colon_name(program_p prog, dict_entry_p de, char *tok) {
  logg("COLON NAME", tok);
  dict_entry_set_name(de, tok);
  return COLON_EXPECTING_SEMI_COLON;
}

static parser_state_t parse_colon_definition(dict_entry_p de, char *tok) {
  logg("COLON_DEF", tok);
  if (strcmp(tok, ";") == 0) {
    dict_add_entry(0, de);
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "DO") == 0) {
    do_create(de->prog);
    return COLON_EXPECTING_SEMI_COLON;
  }
  if (strcmp(tok, "LOOP") == 0) {
    loop_create(de->prog);
    return COLON_EXPECTING_SEMI_COLON;
  }
  if (strcmp(tok, "IF") == 0) {
    if_create(de->prog);
    return COLON_EXPECTING_SEMI_COLON;
  }
  if (strcmp(tok, "ELSE") == 0) {
    else_create(de->prog);
    return COLON_EXPECTING_SEMI_COLON;
  }
  if (strcmp(tok, "THEN") == 0) {
    then_create(de->prog, current_IF);
    return COLON_EXPECTING_SEMI_COLON;
  }
  dict_entry_add_word(de, tok);
  return COLON_EXPECTING_SEMI_COLON;
}

static parser_state_t parse_if(program_p prog, p_code_p cp, char *tok) {
  logg("IF", tok);

  if (strcmp(tok, "ELSE") == 0) {
    else_create(prog);
    return ELSE_EXPECTING_THEN;
  }
  if (strcmp(tok, "THEN") == 0) {
    then_create(prog, current_IF);
    return EXPECTING_ANY;
  }
  parse_word(prog, tok);
  return IF_EXPECTING_ELSE_OR_THEN;
}

static parser_state_t parse_else(program_p prog, p_code_p cp, char *tok) {
  logg("ELSE", tok);
  if (strcmp(tok, "THEN") == 0) {
    then_create(prog, current_IF);
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "ELSE") == 0) {
    else_create(prog);
    return ELSE_EXPECTING_THEN;
  }
  parse_word(prog, tok);
  return ELSE_EXPECTING_THEN;
}

static parser_state_t parse_do(program_p prog, p_code_p saved_do_code,
                               char *tok) {
  logg("DO", tok);
  if (strcmp(tok, "DO") == 0) {
    do_create(prog);
    return DO_EXPECTING_LOOP;
  }
  if (strcmp(tok, "LOOP") == 0) {
    loop_create(prog);
    return EXPECTING_ANY;
  }
  return parse_word(prog, tok);
}

static parser_state_t state = EXPECTING_ANY;
/**
REMEMBER THE STATES ACROSS PROGRAM INFOCATIONS
*/
program_p parse(ftask_p task, char *source) {
  program_p prog = task->program;
  static const char *delim = " \t\n\r";

  for (char *tok = strtok(source, delim); tok; tok = strtok(0, delim)) {
    switch (state) {
    case EXPECTING_ANY:
      state = parse_word(prog, tok);
      break;
    case COLON_EXPECTING_NAME:
      state = parse_colon_name(prog, de, tok);
      break;
    case COLON_EXPECTING_SEMI_COLON:
      state = parse_colon_definition(de, tok);
      break;
    case IF_EXPECTING_ELSE_OR_THEN:
      state = parse_if(prog, current_IF, tok);
      break;
    case ELSE_EXPECTING_THEN:
      state = parse_else(prog, current_IF, tok);
      break;
    case DO_EXPECTING_LOOP:
      state = parse_do(prog, current_DO, tok);
      break;
    case PS_COMMENT:
      state = parse_comment(tok);
      break;
    case VARIABLE_EXPECTING_NAME:
      state = parse_variable(prog, tok);
      break;
    default:
      printf("Illegal token (%s) detected\n", tok);
      exit(-1);
    }
  }
  return prog;
}
