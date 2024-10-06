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
    EXPECTING_ANY,
    PS_COMMENT,
    COLON_EXPECTING_NAME,
    COLON_EXPECTING_SEMI_COLON,
    IF_EXPECTING_ELSE_OR_THEN,
    PS_IF_CODE,
    ELSE_EXPECTING_THEN,
    DO_EXPECTING_LOOP,
    CREATE_EXPECTING_NAME,
    VARIABLE_EXPECTING_NAME,
    PS_ERROR
} parser_state_t;


program_p parse(ftask_p, char*program_text);

#endif /* PARSER_H_ */
