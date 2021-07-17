#ifndef ERROR_H
#define ERROR_H

typedef enum error_code {
// Interpreter errors
EBAD_ARGS = 15, EOPEN_FILE,

// Lexical errors
EBAD_SYMBOL,

// Syntax errors
EBAD_INDENT, EBAD_TOK, EBAD_OP, EBAD_EXPR,
EBAD_COND, ENO_BODY, EBAD_IDX, EBAD_TERM,

// Runtime errors
EDIV_ZERO, EBAD_BREAK, EBAD_CONT
} ErrorCode;

#endif // ERROR_H
