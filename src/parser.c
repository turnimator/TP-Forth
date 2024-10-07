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

p_code_p current_cond;          // Current conditional
p_code_p current_loop; // Current DO

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

static void if_create(program_p prog) {
  ct_push(CT_IF, current_cond); // In case of a nested IF, tuck away the old
  current_cond = p_code_ct_create(PCODE_IF); // and enter the new
  program_add_p_code(prog, current_cond);
}

static void then_create(program_p prog, p_code_p cond) {
  program_set_if_end_marker(prog, cond);
  cond = ct_pop(CT_IF);
}

static void do_create(program_p prog) {
  ct_push(CT_DO, current_loop); // Must be popped on LOOP
  current_loop = p_code_ct_create(PCODE_LOOP_DO);
  current_loop->val.l = program_last(prog); // This is for LOOP to know where to jump back to
  program_add_p_code(prog, current_loop);
}

static void loop_create(program_p prog) {
  logg(prog->name, "ENTER");
  p_code_p loop_code;
  loop_code = p_code_ct_create(PCODE_LOOP_END);
  loop_code->val.l = current_loop->val.l;
  current_loop->val.l = program_last(prog) + 1;
  program_dump(prog, 0);
  program_add_p_code(prog, loop_code);
  current_loop = ct_pop(CT_DO);
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
    then_create(prog, current_cond); // and get back the old one
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
  if (strcmp(tok, ";") == 0) {
    logg(";", "unexpected");
    return EXPECTING_ANY;
  }
  if (isForthCommentStart(tok)) {
    return PS_COMMENT;
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
    current_cond->val.l = program_last(de->prog);
    return COLON_EXPECTING_SEMI_COLON;
  }
  if (strcmp(tok, "THEN") == 0) {
    then_create(de->prog, current_cond);
    return COLON_EXPECTING_SEMI_COLON;
  }
  dict_entry_add_word(de, tok);
  return COLON_EXPECTING_SEMI_COLON;
}

static parser_state_t parse_if(program_p prog, p_code_p cp, char *tok) {
  logg("IF", tok);

  if (strcmp(tok, "ELSE") == 0) {
    current_cond->val.l = program_last(prog);
    return ELSE_EXPECTING_THEN;
  }
  if (strcmp(tok, "THEN") == 0) {
    then_create(prog, current_cond);
    return EXPECTING_ANY;
  }
  return parse_word(prog, tok);
}

static parser_state_t parse_else(program_p prog, p_code_p cp, char *tok) {
  logg("ELSE", tok);
  if (strcmp(tok, "THEN") == 0) {
    then_create(prog, current_cond);
    return EXPECTING_ANY;
  }
  return parse_word(prog, tok);
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
      state = parse_if(prog, current_cond, tok);
      break;
    case ELSE_EXPECTING_THEN:
      state = parse_else(prog, current_cond, tok);
      break;
    case DO_EXPECTING_LOOP:
      state = parse_do(prog, current_loop, tok);
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
