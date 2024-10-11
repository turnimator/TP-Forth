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
    COLON_EXPECTING_NAME,
    CREATE_EXPECTING_NAME,
    VARIABLE_EXPECTING_NAME,
    PS_COMMENT,
    EXPECTING_ANY,
    PS_ERROR
} parser_state_t;


program_p parse(ftask_p, char*program_text);

#endif /* PARSER_H_ */
