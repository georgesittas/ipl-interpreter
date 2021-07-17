#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_type {
	// Assignment & comment
	EQUAL, HASH,

	// Arithmetic operators
	PLUS, MINUS, STAR, SLASH, MODULO,

	// Conditional operators
	EQUAL_EQUAL, BANG_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,

	// Values
	IDENTIFIER, NUMBER,

	// Keywords
	READ, WRITE, WRITELN, IF, ELSE, WHILE, RANDOM, ARGUMENT, SIZE, BREAK, CONTINUE,

	// Special
	TAB, NEWLINE, ENDOFFILE
} TokenType;

typedef struct token {
	TokenType type;
	char* lexeme;
	int literal; // Useful for numbers (slow to call atoi all the time)
	int line; // This comes in handy for error handling
} Token;

#endif // TOKEN_H
