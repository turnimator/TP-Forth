/*
 * parser.c
 *
 *  Created on: Sep 22, 2024
 *      Author: Jan Atle Ramsli
 *
 */
// #define DEBUG

#include "parser.h"

#include <string.h>

#include "compiler.h"
#include "dictionary.h"
#include "logg.h"
#include "p_code.h"
#include "program.h"
#include "task.h"
#include "variable.h"

// IF ELSE THEN
//  IF set FALSE JUMP address to 0
//  ELSE set FALSE JUMP address to HERE+1
//  THEN if FALSE JUMP address is 0, set FALSE JUMP address to HERE+1

p_code_p current_IF; // Current conditional
p_code_p current_DO; // Current DO
p_code_p current_BEGIN;

program_p current_PROG;

static dict_entry_p de = 0; // Current colon definition

/**
 * IF TRUE, fall though. When encountering ELSE skip to THEN
 * IF FALSE skip to ELSE+1 (thereby executing ELSE code) or THEN+1
 */
static void if_create() {
  ct_push(CT_IF, current_IF); // In case of a nested IF, tuck away the old
  current_IF = p_code_ct_create(PCODE_IF, "IF"); // and enter the new
  current_IF->val.l = current_PROG->npcp_array;  // Note where the IF is
  program_add_p_code(current_PROG, current_IF);
}

static void else_create() {

  // current_IF->val.l = current_PROG->npcp_array; // Make IF FALSE point here
  ct_push(CT_IF, current_IF);
  current_IF = p_code_ct_create(PCODE_ELSE, "ELSE"); //
  current_IF->val.l =
      current_PROG->npcp_array; // Mark where we are in the stream
  program_add_p_code(current_PROG, current_IF);
}

/**
Look at the compile time stack.
If the current is ELSE, we must ct_pop and fixup before handling THEN.
// NB: RELATIVE JUMPS:
// Jump to THEN_offset from IF_offset THEN_offset - IF_offset
// JUMP to ELSE_offset from IF_offset ELSE_offset - IF_offset
// JUMP to THEN_offset from ELSE_offset THEN_offset - IF_offset
// ALL OF this has to be done when encountering THEN
**/
static void then_create() {
  long THEN_offset = current_PROG->npcp_array;
  long ELSE_offset = 0;
  long IF_offset = 0;
  if (current_IF->jtidx == PCODE_ELSE) {
    ELSE_offset = current_IF->val.l;
    current_IF->val.l = THEN_offset - ELSE_offset;
    current_IF = ct_pop(CT_IF);
  }
  /* If we had an IF ELSE THEN, we are now on the IF THEN part
  update IF*/
  if (ELSE_offset == 0) {
    IF_offset = current_IF->val.l;
    current_IF->val.l = THEN_offset - IF_offset;
  }
  p_code_p pcode = p_code_ct_create(PCODE_THEN, "THEN"); //
  pcode->val.l = current_PROG->npcp_array;
  program_add_p_code(current_PROG, pcode);
  current_IF = ct_pop(CT_IF);
}

/**

**/
static void do_create() {
  logg("DO", "PUSH");
  ct_push(CT_DO, current_DO); // Must be popped on LOOP
  current_DO = p_code_ct_create(PCODE_LOOP_DO, "DO");
  current_DO->val.l = current_PROG->npcp_array + 1;
  program_add_p_code(current_PROG, current_DO);
}

static void begin_create() {
  logg("BEGIN", "PUSH");
  ct_push(CT_BEGIN, current_BEGIN); // Must be popped on LOOP
  current_BEGIN = p_code_ct_create(PCODE_LOOP_BEGIN, "BEGIN");
  current_BEGIN->val.l = current_PROG->npcp_array + 1;
  program_add_p_code(current_PROG, current_BEGIN);
}

static void loop_create() {
  int ijump;
  ijump = current_DO->val.l;
  logg("CREATE", "ENTER");
  p_code_p loop_end_code;
  loop_end_code = p_code_ct_create(PCODE_LOOP_END, "LOOP");
  loop_end_code->val.l = ijump;
  program_add_p_code(current_PROG, loop_end_code);
  current_DO = ct_pop(CT_DO); // POP back the old loop
  logg("LOOP", "POP");
}

static void again_create() {
  int ijump;
  ijump = current_BEGIN->val.l;
  logg("BEGIN", "AGAIN");
  p_code_p loop_end_code;
  loop_end_code = p_code_ct_create(PCODE_LOOP_AGAIN, "AGAIN");
  loop_end_code->val.l = ijump;
  program_add_p_code(current_PROG, loop_end_code);
  current_BEGIN = ct_pop(CT_BEGIN); // POP back the old loop
  logg("LOOP", "POP");
}


static void exit_create() {
  logg("EXIT", "");
  p_code_p exit_code = p_code_ct_create(PCODE_EXIT, "EXIT");
  program_add_p_code(current_PROG, exit_code);
}

static void string_create(char *s) {
  p_code_p ps = p_code_compile_word(s);
  program_add_p_code(current_PROG, ps);
}

static parser_state_t parse_variable(parser_state_t state, char *name) {
  logg("VARIABLE", name);
  return VARIABLE_EXPECTING_NAME;
}

static parser_state_t parse_variable_name(parser_state_t state, char *name) {
  logg("VARIABLE_EXPECTING_NAME", name);
  if (strcmp(name, "VARIABLE") == 0) {
    printf("parse error variable %s\n", name);
    return VARIABLE_EXPECTING_NAME;
  }
#ifdef DEBUG
  printf("adding variable %s\n", name);
#endif
  var_p vp = variable_add(name);
#ifdef DEBUG
  printf("var_p index=%d", vp->vt_idx);
#endif
  return EXPECTING_ANY;
}

char string_buffer[1024];

/**
ALL ACTIONS MUST BE PERFORMED HERE
*/
parser_state_t parse_word(parser_state_t state, char *tok) {
  logg("WORD", tok);
  if (tok[0] == '\"') {
    strcpy(string_buffer, tok);
    return STRING_EXPECTING_QUOTE;
  }
  if (strcmp(tok, ":") == 0) {
    return COLON_EXPECTING_NAME;
  }
  if (strcmp(tok, "IF") == 0) {
    if_create();
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "THEN") == 0) {
    then_create();
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "ELSE") == 0) {
    else_create();
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "DO") == 0) {
    do_create();
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "LOOP") == 0) {
    loop_create();
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "BEGIN") == 0) {
    begin_create();
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "AGAIN") == 0) {
    again_create();
    return EXPECTING_ANY;
  }

  if (strcmp(tok, "EXIT") == 0) {
    exit_create();
    return EXPECTING_ANY;
  }

  if (strcmp(tok, "VARIABLE") == 0) {
    return VARIABLE_EXPECTING_NAME;
  }
  if (strcmp(tok, ";") ==
      0) { // will crash the system if not in colon definition
    current_PROG = ct_prog_pop();
#ifdef DEBUG
    // dict_dump(0);
#endif
    return EXPECTING_ANY;
  }
  if (strcmp(tok, "(") == 0) {
    return PAREN_COMMENT_EXPECTING_RIGHT_PAREN;
  }

  if (strcmp(tok, "\\") == 0) {
    return BACKSPACE_COMMENT_EXPECTING_NEWLINE;
  }

  program_add(current_PROG, tok);
  return EXPECTING_ANY;
}

parser_state_t parse_comment_look_for_right_paren(parser_state_t state,
                                                  char *tok) {
  logg("PS COMMENT", tok);
  if (strcmp(tok, ")") == 0) {
    return EXPECTING_ANY;
  }
  return PAREN_COMMENT_EXPECTING_RIGHT_PAREN;
}

/**
 This will not work because strtok swallows newlines.
 This must be caucht before strtok gets it.
*/
parser_state_t parse_comment_look_for_newline(parser_state_t state, char *tok) {
  logg("BS COMMENT", tok);
  if (strcmp(tok, "\n") == 0) {
    return EXPECTING_ANY;
  }
  return BACKSPACE_COMMENT_EXPECTING_NEWLINE;
}

parser_state_t parse_colon(parser_state_t state, char *tok) {
  logg("COLON", tok);
  return COLON_EXPECTING_NAME;
}

/**
The action is performed by the caller
*/
parser_state_t parse_colon_name(parser_state_t state, char *tok) {
  logg("COLON NAME", tok);
  de = dict_entry_create(0, tok);
  ct_prog_push(current_PROG);
  current_PROG = de->prog;

  dict_add_entry(0, de);
#ifdef DEBUG
  //(0);
#endif
  return EXPECTING_ANY;
}

parser_state_t parse_error(parser_state_t state, char *tok) {
  printf("PARSE ERROR %s\n", tok);
  return EXPECTING_ANY;
}

parser_state_t parse_string_expecting_quote(parser_state_t state, char *tok) {
  logg("Waiting for \"", tok);
  if (tok[strlen(tok) - 1] == '\"') {
    strcat(string_buffer, tok);
    string_create(string_buffer);
    return EXPECTING_ANY;
  } else {
    strcat(string_buffer, tok);
    strcat(string_buffer, " ");
    return STRING_EXPECTING_QUOTE;
  }
}

static parser_state_t state = EXPECTING_ANY;

typedef parser_state_t (*parse_func)(parser_state_t state, char *);
typedef int (*cmpfunc)(const char *, const char *);

static inline int cmpany(const char *s1, const char *s2) { return 0; }

typedef struct parse_table_entry {
  char *tok;
  cmpfunc cmpf;
  parse_func f;
} ptentry_t, *ptentry_p;

/*
When making changes, remember to copy from parser.h and paste here:
   COLON = 0,
    COLON_EXPECTING_NAME = 1,
    CREATE = 2,
    CREATE_EXPECTING_NAME =3,
    VARIABLE = 4,
    VARIABLE_EXPECTING_NAME = 5,
    PAREN_COMMENT_EXPECTING_RIGHT_PAREN =6,
    BACKSPACE_COMMENT_EXPECTING_NEWLINE = 7,
    EXPECTING_ANY = 8,
    PS_ERROR = 9,
    STRING_EXPECTING_QUOTE=10
*/
static ptentry_t pftable[] = {{":", strcmp, parse_colon},
                              {"", cmpany, parse_colon_name},
                              {"CREATE", strcmp, parse_variable},
                              {"", cmpany, parse_variable_name},
                              {"VARIABLE", strcmp, parse_variable},
                              {"", cmpany, parse_variable_name},
                              {"(", strcmp, parse_comment_look_for_right_paren},
                              {"\\", strcmp, parse_comment_look_for_newline},
                              {"", cmpany, parse_word},
                              {"", cmpany, parse_error},
                              {"", cmpany, parse_string_expecting_quote}};

/**

*/
program_p parse(ftask_p task, char *source) {
  static const char *delim = " \t\n\r";
  state = EXPECTING_ANY; // entry no. 4 in the table (the "default")

  current_PROG = program_create("MAIN");
  task->program = current_PROG;
#ifdef DEBUG
  printf("Program before parse:\n");
  program_dump(current_PROG, task);
#endif

  /*
  for each token in the input look up the parse table entry corresponding to the
  current state (the entry no in the table) feed the current token to the
  table's compare function (cmpfunc) if the compare function returns 0, execute
  the corresponding parse function (parse_func)
  */
  for (char *tok = strtok(source, delim); tok; tok = strtok(0, delim)) {
    if (*tok == '\\') {
      tok++;
      while (*tok != '\n') {
        tok++;
      }
      source = tok;
      tok = strtok(source, delim);
      state = EXPECTING_ANY;
    }
    if (strcmp(tok, "(") == 0) {
      tok++;
      while (*tok != ')') {
        tok++;
      }
      source = tok + 1;
      tok = strtok(source, delim);
      state = EXPECTING_ANY;
    }
    if (pftable[state].cmpf(tok, pftable[state].tok) == 0) {
      state = pftable[state].f(state, tok);
    }
  }
#ifdef DEBUG
  printf("\nProgram after parse:\n");
  program_dump(current_PROG, task);
#endif
  return current_PROG;
}
