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

// Helper functions used by the scanner (no reason to expose them)
static int* create_int(int value);
static void install_keywords(void);
static int identifier_token(char* lexeme);
static void destroy_token(void* item);
static void scan_token(void);
static void identifier(void);
static void number(void);
static void consume_symbol(int symbol);
static bool match_symbol(int symbol);
static void add_token(TokenType type, char* lexeme, int literal);
static bool is_alpha(int symbol);
static bool is_digit(int symbol);
static bool is_alnum(int symbol);
static bool reached_eof(void);

// Scanner state is maintained with the help of the following globals
static Vector tokens;

static FILE* stream;
static Map keywords; // Implements the mapping keyword -> type

static int line = 1;
static int lexeme_pos = 0;
static char lexeme[MAX_LEXEME+1];

static bool computing_indentation = true;

static int* create_int(int value) {
	int* new_int = malloc(sizeof(int));
	assert(new_int != NULL);
	*new_int = value;
	return new_int;
}

static void install_keywords(void) {
	keywords = map_create(NULL, NULL, free, NULL);

	map_put(keywords, "read", create_int(READ));
	map_put(keywords, "write", create_int(WRITE));
	map_put(keywords, "writeln", create_int(WRITELN));
	map_put(keywords, "if", create_int(IF));
	map_put(keywords, "else", create_int(ELSE));
	map_put(keywords, "while", create_int(WHILE));
	map_put(keywords, "random", create_int(RANDOM));
	map_put(keywords, "argument", create_int(ARGUMENT));
	map_put(keywords, "size", create_int(SIZE));
	map_put(keywords, "break", create_int(BREAK));
	map_put(keywords, "continue", create_int(CONTINUE));
	map_put(keywords, "new", create_int(NEW));
	map_put(keywords, "free", create_int(FREE));
}

static int identifier_token(char* lexeme) {
	int* keyword_type = map_get(keywords, lexeme);
	return keyword_type == NULL ? IDENTIFIER : *keyword_type;
}

static void destroy_token(void* item) {
	Token* token = (Token*) item;
	free(token->lexeme);
	free(token);
}

Vector scan_tokens(FILE* fp) {
	stream = fp;
	tokens = vector_create(destroy_token);

	install_keywords();
	while (!reached_eof()) {
		lexeme_pos = 0;
		scan_token();
	}

	add_token(ENDOFFILE, "<EOF>", 0);

	map_destroy(keywords);
	return tokens;
}

static void scan_token(void) {
	int symbol = fgetc(stream);

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
			if (computing_indentation) {
				add_token(TAB, "\\t", 0);
			}
			return;

		case '#':
			// Skip comments completely (falls through to case '\n' on purpose)
			while (fgetc(stream) != '\n') {
				if (reached_eof()) {
					return;
				}
			}

		case '\n':
			computing_indentation = true;
			add_token(NEWLINE, "\\n", 0);
			line++;
			return;

		case ' ':
			break; // Ignore spaces completely

		default:
			lexeme[lexeme_pos++] = symbol;
			if (is_alpha(symbol)) {
				identifier();
			} else if (is_digit(symbol)) {
				number();
			} else if (symbol != EOF) {
				fprintf(stderr, "Lexical Error: unexpected character at line %d - %c\n", line, symbol);
				exit(EBAD_SYMBOL);
			}
			break;
	}

	computing_indentation = false;
}

static void identifier(void) {
	int symbol;

	while (!reached_eof()) {
		symbol = fgetc(stream);
		if (!is_alnum(symbol)) {
			ungetc(symbol, stream);
			break;
		}

		lexeme[lexeme_pos++] = symbol;
	}

	lexeme[lexeme_pos] = '\0';
	add_token(identifier_token(lexeme), lexeme, 0);
}

static void number(void) {
	int symbol;

	while (!reached_eof()) {
		symbol = fgetc(stream);
		if (!is_digit(symbol)) {
			ungetc(symbol, stream);
			break;
		}

		lexeme[lexeme_pos++] = symbol;
	}

	lexeme[lexeme_pos] = '\0';
	add_token(NUMBER, lexeme, atoi(lexeme));
}

static void consume_symbol(int symbol) {
	int ch = fgetc(stream);

	if (ch != symbol) {
		fprintf(stderr, "Lexical Error: unexpected character at line %d - %c\n", line, ch);
		exit(EBAD_SYMBOL);
	}
}

static bool match_symbol(int symbol) {
	int ch = fgetc(stream);

	if (ch == symbol)
		return true;

	ungetc(ch, stream);
	return false;
}

static void add_token(TokenType type, char* lexeme, int literal) {
	Token* token = malloc(sizeof(Token));
	assert(token != NULL);

	token->lexeme = (lexeme == NULL) ? NULL : strdup(lexeme);
	assert(lexeme == NULL || token->lexeme != NULL);

	token->type = type;
	token->literal = literal;
	token->line = line;

	vector_add(tokens, token);
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
	return feof(stream);
}
