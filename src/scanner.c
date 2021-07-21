#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "map.h"
#include "vector.h"

#include "error.h"
#include "token.h"
#include "scanner.h"

#define MAX_LEXEME 100

// Helper functions used by the scanner (no reason to expose them)
static void init_scanner(FILE* fp);
static void scan_token(void);
static void scan_identifier(void);
static void scan_number(void);
static void consume_symbol(int symbol);
static bool match_symbol(int symbol);
static void add_token(TokenType type, char* lexeme, int literal);
static bool is_alpha(int symbol);
static bool is_digit(int symbol);
static bool is_alnum(int symbol);
static bool reached_eof(void);

// This is used as a wrapper for the scanner's state
static struct scanner {
	Vector tokens;
	FILE* stream;
	Map keywords;
	int line;
	int lexeme_pos;
	char lexeme[MAX_LEXEME+1];
	int current_indentation;
	bool computing_indentation;
	bool currently_at_blank_line;
} scanner;

static int* create_int(int value) {
	int* new_int = malloc(sizeof(int));
	assert(new_int != NULL);
	*new_int = value;
	return new_int;
}

static void destroy_token(void* item) {
	Token* token = (Token*) item;
	free(token->lexeme);
	free(token);
}

static void init_scanner(FILE* fp) {
	scanner.stream = fp;
	scanner.tokens = vector_create(destroy_token);

	scanner.line = 1;
	scanner.lexeme_pos = 0;
	scanner.current_indentation = 0;
	scanner.computing_indentation = true;
	scanner.currently_at_blank_line = true;

	scanner.keywords = map_create(NULL, NULL, free, NULL);

	map_put(scanner.keywords, "read", create_int(READ));
	map_put(scanner.keywords, "write", create_int(WRITE));
	map_put(scanner.keywords, "writeln", create_int(WRITELN));
	map_put(scanner.keywords, "if", create_int(IF));
	map_put(scanner.keywords, "else", create_int(ELSE));
	map_put(scanner.keywords, "while", create_int(WHILE));
	map_put(scanner.keywords, "random", create_int(RANDOM));
	map_put(scanner.keywords, "argument", create_int(ARGUMENT));
	map_put(scanner.keywords, "size", create_int(SIZE));
	map_put(scanner.keywords, "break", create_int(BREAK));
	map_put(scanner.keywords, "continue", create_int(CONTINUE));
	map_put(scanner.keywords, "new", create_int(NEW));
	map_put(scanner.keywords, "free", create_int(FREE));
}

Vector scan_tokens(FILE* fp) {
	init_scanner(fp);

	while (!reached_eof()) {
		scanner.lexeme_pos = 0;
		scan_token();
	}

	add_token(ENDOFFILE, "<EOF>", 0);

	map_destroy(scanner.keywords);
	return scanner.tokens;
}

static void scan_token(void) {
	int symbol = fgetc(scanner.stream);

	switch (symbol) {
		case '+': add_token(PLUS, "+", 0); break;
		case '-': add_token(MINUS, "-", 0); break;
		case '*': add_token(STAR, "*", 0); break;
		case '/': add_token(SLASH, "/", 0); break;
		case '%': add_token(MODULO, "%", 0); break;
		case '[': add_token(LSBRACE, "[", 0); break;
		case ']': add_token(RSBRACE, "]", 0); break;

		case '!':
			consume_symbol('=');
			add_token(BANG_EQUAL, "!=", 0);
			break;

		case '=':
			if (match_symbol('=')) {
				add_token(EQUAL_EQUAL, "==", 0);
			} else {
				add_token(EQUAL, "=", 0);
			}
			break;

		case '<':
			if (match_symbol('=')) {
				add_token(LESS_EQUAL, "<=", 0);
			} else {
				add_token(LESS, "<", 0);
			}
			break;

		case '>':
			if (match_symbol('=')) {
				add_token(GREATER_EQUAL, ">=", 0);
			} else {
				add_token(GREATER, ">", 0);
			}
			break;

		case '\t':
			if (scanner.computing_indentation) {
				scanner.current_indentation++;
			}
			return;

		case '#':
			// Skip comments completely (falls through to case '\n' on purpose)
			while (fgetc(scanner.stream) != '\n') {
				if (reached_eof()) {
					return;
				}
			}

		case '\n':
			if (!scanner.currently_at_blank_line) {
				add_token(NEWLINE, "\\n", 0); // No need to add tokens for empty lines
			}

			scanner.line++;
			scanner.current_indentation = 0;
			scanner.computing_indentation = true;
			scanner.currently_at_blank_line = true;
			return;

		case ' ':
			break; // Ignore spaces completely

		default:
			scanner.lexeme[scanner.lexeme_pos++] = symbol;
			if (is_alpha(symbol)) {
				scanner.currently_at_blank_line = false; // Non-blank lines start with an identifier

				if (scanner.computing_indentation) {
					while (scanner.current_indentation--) {
						add_token(TAB, "\\t", 0); // Add the tabs we counted earlier
					}
				}

				scan_identifier();
			} else if (is_digit(symbol)) {
				scan_number();
			} else if (symbol != EOF) {
				fprintf(stderr, "Lexical Error: unexpected character '%c' at line %d\n",
					symbol, scanner.line);
				exit(EBAD_SYMBOL);
			}
			break;
	}

	scanner.computing_indentation = false;
}

static void scan_identifier(void) {
	int symbol;

	while (!reached_eof()) {
		symbol = fgetc(scanner.stream);
		if (!is_alnum(symbol)) {
			ungetc(symbol, scanner.stream);
			break;
		}

		scanner.lexeme[scanner.lexeme_pos++] = symbol;
	}

	scanner.lexeme[scanner.lexeme_pos] = '\0';

	int* keyword_type = map_get(scanner.keywords, scanner.lexeme);
	add_token(keyword_type == NULL ? IDENTIFIER : *keyword_type, scanner.lexeme, 0);
}

static void scan_number(void) {
	int symbol;

	while (!reached_eof()) {
		symbol = fgetc(scanner.stream);
		if (!is_digit(symbol)) {
			ungetc(symbol, scanner.stream);
			break;
		}

		scanner.lexeme[scanner.lexeme_pos++] = symbol;
	}

	scanner.lexeme[scanner.lexeme_pos] = '\0';
	add_token(NUMBER, scanner.lexeme, atoi(scanner.lexeme));
}

static void consume_symbol(int symbol) {
	int ch = fgetc(scanner.stream);

	if (ch != symbol) {
		fprintf(stderr, "Lexical Error: unexpected character '%c' at line %d\n",
			ch, scanner.line);
		exit(EBAD_SYMBOL);
	}
}

static bool match_symbol(int symbol) {
	int ch = fgetc(scanner.stream);

	if (ch == symbol)
		return true;

	ungetc(ch, scanner.stream);
	return false;
}

static void add_token(TokenType type, char* lexeme, int literal) {
	Token* token = malloc(sizeof(Token));
	assert(token != NULL);

	token->lexeme = (lexeme == NULL) ? NULL : strdup(lexeme);
	assert(lexeme == NULL || token->lexeme != NULL);

	token->type = type;
	token->literal = literal;
	token->line = scanner.line;

	vector_add(scanner.tokens, token);
}

static bool is_alpha(int symbol) {
	return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
}

static bool is_digit(int symbol) {
	return symbol >= '0' && symbol <= '9';
}

static bool is_alnum(int symbol) {
	return is_alpha(symbol) || is_digit(symbol) || symbol == '_';
}

static bool reached_eof(void) {
	return feof(scanner.stream);
}
