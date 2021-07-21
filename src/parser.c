#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "vector.h"

#include "stmt.h"
#include "expr.h"
#include "error.h"
#include "token.h"
#include "parser.h"

// Helper functions used by the parser (no reason to expose them)
static void init_parser(Vector tokens);
static void parse_stmt(void);
static void parse_read_stmt(int line);
static void parse_assignment_stmt(int line);
static void parse_write_stmt(int line);
static void parse_writeln_stmt(int line);
static void parse_while_stmt(int line, int indent);
static void parse_if_else_stmt(int line, int indent);
static Vector parse_block_stmt(int line, int indent);
static void parse_random_stmt(int line);
static void parse_arg_size_stmt(int line);
static void parse_arg_stmt(int line);
static void parse_break_stmt(int line);
static void parse_continue_stmt(int line);
static void parse_new_stmt(int line);
static void parse_free_stmt(int line);
static void parse_size_stmt(int line);
static Expr* parse_expr(void);
static Expr* parse_rvalue(void);
static Expr* parse_lvalue(void);
static Token* advance_token(void);
static Token* peek_token(void);
static Token* previous_token(void);
static Token* consume_token(TokenType type, bool endable);
static bool match_token(TokenType type);
static int compute_indentation(void);
static bool is_operator(TokenType type);
static bool is_arithm_operator(TokenType type);
static bool is_comp_operator(TokenType type);
static bool reached_end(void);
static void syntax_error(char* msg, int line, int status);

// This is used as a wrapper for the parser's state
static struct parser {
	int curr_token;
	Vector token_stream;
	Vector stmts;
	int curr_indent;
	bool return_from_block;
} parser;

static void init_parser(Vector tokens) {
	parser.token_stream = tokens;

	parser.curr_token = 0;
	parser.curr_indent = 0;
	parser.return_from_block = false;
}

Vector parse(Vector tokens) {
	// This routine is used recursively and we only want init to be called once
	static bool initialized = false;

	if (!initialized) {
		init_parser(tokens);
		initialized = true;
	}

	parser.stmts = vector_create(destroy_stmt);
	while (!reached_end() && !parser.return_from_block) {
		parse_stmt();
	}

	return parser.stmts;
}

static void parse_stmt(void) {
	int temp_token_pos = parser.curr_token; // Keep this in case we need to rewind

	int indent = compute_indentation();
	if (indent != parser.curr_indent) {
		if (indent > parser.curr_indent) {
			syntax_error("invalid indentation", previous_token()->line, EBAD_INDENT);
		}

		// Rewind the stream index to parse the current statement in the proper context
		parser.curr_token = temp_token_pos;

		parser.return_from_block = true;
		return; // End of block
	}

	Token* token = advance_token();
	switch (token->type) {
		case READ: parse_read_stmt(token->line); break;
		case IDENTIFIER: parse_assignment_stmt(token->line); break;
		case WRITE: parse_write_stmt(token->line); break;
		case WRITELN: parse_writeln_stmt(token->line); break;
		case WHILE: parse_while_stmt(token->line, indent); break;
		case IF: parse_if_else_stmt(token->line, indent); break;
		case RANDOM: parse_random_stmt(token->line); break;
		case BREAK: parse_break_stmt(token->line); break;
		case CONTINUE: parse_continue_stmt(token->line); break;
		case NEW: parse_new_stmt(token->line); break;
		case FREE: parse_free_stmt(token->line); break;
		case SIZE: parse_size_stmt(token->line); break;

		case ARGUMENT:
			if (match_token(SIZE)) {
				parse_arg_size_stmt(token->line);
			} else {
				parse_arg_stmt(token->line);
			}
			break;

		case NEWLINE:
		case ENDOFFILE:
			break;

		default:
			syntax_error("unrecognized token", token->line, EBAD_TOK);
	}
}

static void parse_read_stmt(int line) {
	Expr* lvalue = parse_lvalue();
	consume_token(NEWLINE, true);

	ReadStmt* read_stmt = create_read_stmt(lvalue->type == ARRAY, lvalue->expr);
	vector_add(parser.stmts, create_stmt(line, READ_STMT, read_stmt));
}

static void parse_assignment_stmt(int line) {
	parser.curr_token--; // Unread one token so we can begin parsing an lvalue

	Expr* lvalue = parse_lvalue();
	consume_token(EQUAL, false);
	Expr* rhs_expr = parse_expr();

	if (rhs_expr->type == BINARY &&
		  !is_arithm_operator(((Binary*) rhs_expr->expr)->type)) {
		syntax_error("invalid operator in binary expression", line, EBAD_OP);
	}

	consume_token(NEWLINE, true);
	AssignmentStmt* assignment_stmt = create_assignment_stmt(
		lvalue->type == ARRAY, lvalue->expr, rhs_expr
	);

	vector_add(parser.stmts, create_stmt(line, ASSIGNMENT_STMT, assignment_stmt));
}

static void parse_write_stmt(int line) {
	if (peek_token()->type == NEWLINE) {
		consume_token(NEWLINE, true);
		vector_add(parser.stmts, create_stmt(line, WRITE_STMT, create_write_stmt(NULL)));
	} else {
		Expr* write_expr = parse_rvalue();
		consume_token(NEWLINE, true);

		WriteStmt* write_stmt = create_write_stmt(write_expr);
		vector_add(parser.stmts, create_stmt(line, WRITE_STMT, write_stmt));
	}
}

static void parse_writeln_stmt(int line) {
	if (peek_token()->type == NEWLINE) {
		consume_token(NEWLINE, true);
		vector_add(parser.stmts, create_stmt(line, WRITELN_STMT, create_writeln_stmt(NULL)));
	} else {
		Expr* writeln_expr = parse_rvalue();
		consume_token(NEWLINE, true);

		WritelnStmt* writeln_stmt = create_writeln_stmt(writeln_expr);
		vector_add(parser.stmts, create_stmt(line, WRITELN_STMT, writeln_stmt));
	}
}

static void parse_while_stmt(int line, int indent) {
	Expr* cond = parse_expr();
	if (cond->type != BINARY ||
		  !is_comp_operator(((Binary*) cond->expr)->type) ) {
		syntax_error("invalid conditional in while statement", line, EBAD_COND);
	}

	consume_token(NEWLINE, false);

	WhileStmt* while_stmt = create_while_stmt(cond, parse_block_stmt(line, indent));
	vector_add(parser.stmts, create_stmt(line, WHILE_STMT, while_stmt));
}

static void parse_if_else_stmt(int line, int indent) {
	Expr* cond = parse_expr();
	if (cond->type != BINARY ||
		  !is_comp_operator(((Binary*) cond->expr)->type) ) {
		syntax_error("invalid conditional in if-else statement", line, EBAD_COND);
	}

	consume_token(NEWLINE, false);

	Vector then_stmts = parse_block_stmt(line, indent);
	Vector else_stmts = NULL;

	int temp_curr_token = parser.curr_token;
	int next_indent = compute_indentation();

	if (next_indent != indent) { // Next statement can only match with an outer block
		// Fix the stream index to read the current statement in the proper context
		parser.curr_token = temp_curr_token;

		IfElseStmt* if_else_stmt = create_if_else_stmt(cond, then_stmts, else_stmts);
		vector_add(parser.stmts, create_stmt(line, IF_ELSE_STMT, if_else_stmt));
		return; // End of if statement
	}

	if (match_token(ELSE)) {
		int else_line = consume_token(NEWLINE, false)->line;
		else_stmts = parse_block_stmt(else_line, indent);
	} else {
		// Fix the stream index to read the current statement in the proper context
		parser.curr_token = temp_curr_token;
	}

	IfElseStmt* if_else_stmt = create_if_else_stmt(cond, then_stmts, else_stmts);
	vector_add(parser.stmts, create_stmt(line, IF_ELSE_STMT, if_else_stmt));
}

static Vector parse_block_stmt(int line, int indent) {
	int temp_curr_indent = parser.curr_indent;
	parser.curr_indent = indent + 1;

	// We rely on the program's runtime stack to parse a block recursively and
	// make a new vector containing the statements it contains

	Vector curr_stmts = parser.stmts;
	Vector block_stmts = parse(parser.token_stream);

	// Get the state to where it was before parse()
	parser.stmts = curr_stmts;
	parser.curr_indent = temp_curr_indent;

	parser.return_from_block = false;

	if (vector_size(block_stmts) == 0) {
		syntax_error("empty body statement", line, ENO_BODY);
	}
	return block_stmts;
}

static void parse_random_stmt(int line) {
	Expr* lvalue = parse_lvalue();
	consume_token(NEWLINE, true);

	RandomStmt* random_stmt = create_random_stmt(lvalue->type == ARRAY, lvalue->expr);
	vector_add(parser.stmts, create_stmt(line, RANDOM_STMT, random_stmt));
}

static void parse_arg_size_stmt(int line) {
	Expr* lvalue = parse_lvalue();
	consume_token(NEWLINE, true);

	ArgSizeStmt* arg_size_stmt = create_arg_size_stmt(lvalue->type == ARRAY, lvalue->expr);
	vector_add(parser.stmts, create_stmt(line, ARG_SIZE_STMT, arg_size_stmt));
}

static void parse_arg_stmt(int line) {
	Expr* index_expr = parse_rvalue();
	Expr* lvalue = parse_lvalue();
	consume_token(NEWLINE, true);

	ArgStmt* arg_stmt = create_arg_stmt(index_expr, lvalue->type == ARRAY, lvalue->expr);
	vector_add(parser.stmts, create_stmt(line, ARG_STMT, arg_stmt));
}

static void parse_break_stmt(int line) {
	int n_loops = 1;

	if (peek_token()->type == NUMBER) {
		n_loops = consume_token(NUMBER, false)->literal;
		if (n_loops == 0) {
			syntax_error("invalid loop count in break statement", line, EBAD_LOOPS);
		}
	}

	consume_token(NEWLINE, true);
	vector_add(parser.stmts, create_stmt(line, BREAK_STMT, create_break_stmt(n_loops)));
}

static void parse_continue_stmt(int line) {
	int n_loops = 1;

	if (peek_token()->type == NUMBER) {
		n_loops = consume_token(NUMBER, false)->literal;
		if (n_loops == 0) {
			syntax_error("invalid loop count in continue statement", line, EBAD_LOOPS);
		}
	}

	consume_token(NEWLINE, true);
	vector_add(parser.stmts, create_stmt(line, CONTINUE_STMT,
		create_continue_stmt(n_loops)));
}

static void parse_new_stmt(int line) {
	Token* id_token = consume_token(IDENTIFIER, false);
	consume_token(LSBRACE, false);
	Expr* idx_expr = parse_rvalue();
	consume_token(RSBRACE, false);
	consume_token(NEWLINE, true);

	NewStmt* new_stmt = create_new_stmt(id_token->lexeme, idx_expr);
	vector_add(parser.stmts, create_stmt(line, NEW_STMT, new_stmt));
}

static void parse_free_stmt(int line) {
	Token* id_token = consume_token(IDENTIFIER, false);
	consume_token(NEWLINE, true);

	vector_add(parser.stmts, create_stmt(line, FREE_STMT,
		create_free_stmt(id_token->lexeme)));
}

static void parse_size_stmt(int line) {
	Token* id_token = consume_token(IDENTIFIER, false);
	Expr* lvalue = parse_lvalue();
	consume_token(NEWLINE, true);

	SizeStmt* size_stmt = create_size_stmt(
		id_token->lexeme, lvalue->type == ARRAY, lvalue->expr
	);

	vector_add(parser.stmts, create_stmt(line, SIZE_STMT, size_stmt));
}

static Expr* parse_expr(void) {
	Expr* left = parse_rvalue();
	Token* tok = peek_token();

	if (is_operator(tok->type)) {
		advance_token(); // Consume the operator
		Expr* right = parse_rvalue();
		return create_expr(BINARY, create_binary(tok->type, left, right));
	} else {
		return left;
	}
}

static Expr* parse_rvalue(void) {
	Token* curr = advance_token();

	switch (curr->type) {
		case IDENTIFIER:
			if (match_token(LSBRACE)) {
				Expr* idx_expr = parse_rvalue();
				consume_token(RSBRACE, false);
				return create_expr(ARRAY, create_array(curr->lexeme, idx_expr));
			} else {
				return create_expr(VAR, create_var(curr->lexeme));
			}

		case NUMBER:
			return create_expr(LITERAL, create_literal(curr->literal));

		default:
			syntax_error("expected name or literal", curr->line, EBAD_EXPR);
			return NULL; // Unreachable -- silences non-void function warning
	}
}

static Expr* parse_lvalue(void) {
	Expr* lvalue = parse_rvalue();

	if (lvalue->type == LITERAL || lvalue->type == BINARY) {
		syntax_error("expected lvalue", previous_token()->line, EBAD_EXPR);
	}

	return lvalue;
}

static Token* advance_token(void) {
	Token* token = vector_get(parser.token_stream, parser.curr_token);
	if (!reached_end()) {
		parser.curr_token++;
	}
	return token;
}

static Token* peek_token(void) {
	return vector_get(parser.token_stream, parser.curr_token);
}

static Token* previous_token(void) {
	assert(parser.curr_token > 0);
	return vector_get(parser.token_stream, parser.curr_token-1);
}

static Token* consume_token(TokenType type, bool endable) {
	Token* curr = advance_token();

	if (curr->type == ENDOFFILE) {
		if (endable) {
			return curr;
		} else {
			syntax_error("unexpected program termination", curr->line, EBAD_TERM);
		}
	} else if (curr->type != type) {
		syntax_error("unexpected token", curr->line, EBAD_TOK);
	}

	return curr;
}

static bool match_token(TokenType type) {
	if (peek_token()->type != type) {
		return false;
	} else {
		if (!reached_end()) {
			parser.curr_token++;
		}
		return true;
	}
}

static int compute_indentation(void) {
	int indent = 0;

	while (match_token(TAB)) {
		indent++;
	}

	return indent;
}

static bool is_operator(TokenType type) {
	return is_arithm_operator(type) || is_comp_operator(type);
}

static bool is_arithm_operator(TokenType type) {
	return type == PLUS   || type == MINUS   || type == STAR ||
	       type == SLASH  || type == MODULO;
}

static bool is_comp_operator(TokenType type) {
	return type == EQUAL_EQUAL || type == BANG_EQUAL || type == LESS           ||
	       type == LESS_EQUAL  || type == GREATER    || type == GREATER_EQUAL;
}

static bool reached_end(void) {
	Token* curr_tok = vector_get(parser.token_stream, parser.curr_token);
	return curr_tok->type == ENDOFFILE;
}

static void syntax_error(char* msg, int line, int status) {
	fprintf(stderr, "Syntax Error: %s at line %d\n", msg, line);
	exit(status);
}
