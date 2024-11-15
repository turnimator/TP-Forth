/*
 * fnf_compiler.h
 *
 *  Created on: Sep 22, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "runtime.h"
#define MAX_WORD_LEN 32
#define MAX_DEFINITIONS 1024
#define MAX_WORDS_IN_DEF 256

typedef enum parser_state {
  COLON = 0,
  COLON_EXPECTING_NAME = 1,
  CREATE = 2,
  CREATE_EXPECTING_NAME = 3,
  VARIABLE = 4,
  VARIABLE_EXPECTING_NAME = 5,
  PAREN_COMMENT_EXPECTING_RIGHT_PAREN = 6,
  BACKSPACE_COMMENT_EXPECTING_NEWLINE = 7,
  EXPECTING_ANY = 8,
  PS_ERROR = 9,
  STRING_EXPECTING_QUOTE = 10,

} parser_state_t;

program_p parse(ftask_p, char *program_text);

#endif /* PARSER_H_ */
