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

// Helper functions used by the scanner (no reason to expose them)
static void parse_stmt(void);
static void read_stmt(int line);
static void assignment_stmt(int line);
static void write_stmt(int line);
static void writeln_stmt(int line);
static void while_stmt(int line, int indent);
static void if_else_stmt(int line, int indent);
static Vector block_stmt(int line, int indent);
static void random_stmt(int line);
static void arg_size_stmt(int line);
static void arg_stmt(int line);
static void break_stmt(int line);
static void continue_stmt(int line);
static Expr* expr(void);
static void var_or_literal(Token* token, void** expr, ExprType* type);
static Token* advance(void);
static Token* peek(void);
static Token* previous(void);
static Token* consume_token(TokenType type, bool endable);
static bool match_token(TokenType type);
static int compute_indentation(void);
static bool is_operator(TokenType type);
static bool is_arithm_operator(TokenType type);
static bool is_comp_operator(TokenType type);
static bool reached_end(void);
static void syntax_error(char* msg, int line, int status);

// Parser state is maintained with the help of the following globals
static int curr_token = 0;
static Vector token_stream;
static Vector stmts;

static int curr_indent = 0;
static bool return_from_block = false;

Vector parse(Vector tokens) {
	token_stream = tokens;
	stmts = vector_create(destroy_stmt);

	while (!reached_end() && !return_from_block) {
		parse_stmt();
	}

	return stmts;
}

static void parse_stmt(void) {
	int temp_token_pos = curr_token; // Keep this in case we need to rewind (see below)

	int indent = compute_indentation();
	if (indent != curr_indent) {
		if (indent > curr_indent) {
			syntax_error("invalid indentation", previous()->line, EBAD_INDENT);
		}

		// Rewind the stream index to parse the current statement in the proper context
		curr_token = temp_token_pos;

		return_from_block = true;
		return; // End of block
	}

	Token* token = advance();
	switch (token->type) {
		case READ: read_stmt(token->line); break;
		case IDENTIFIER: assignment_stmt(token->line); break;
		case WRITE: write_stmt(token->line); break;
		case WRITELN: writeln_stmt(token->line); break;
		case WHILE: while_stmt(token->line, indent); break;
		case IF: if_else_stmt(token->line, indent); break;
		case RANDOM: random_stmt(token->line); break;
		case BREAK: break_stmt(token->line); break;
		case CONTINUE: continue_stmt(token->line); break;

		case ARGUMENT:
			if (match_token(SIZE)) {
				arg_size_stmt(token->line);
			} else {
				arg_stmt(token->line);
			}
			break;

		case NEWLINE:
		case ENDOFFILE:
			break;

		default:
			syntax_error("unrecognized token", token->line, EBAD_TOK);
	}
}

static void read_stmt(int line) {
	Token* var_token = consume_token(IDENTIFIER, false);
	consume_token(NEWLINE, true);

	ReadStmt* readstmt = create_readstmt(create_var(var_token->lexeme, 0));
	vector_add(stmts, create_stmt(line, READ_STMT, readstmt));
}

static void assignment_stmt(int line) {
	Token* lvalue_token = previous();
	consume_token(EQUAL, false);
	Expr* rhs_expr = expr();

	if (rhs_expr->type == BINARY &&
		  !is_arithm_operator(((Binary*) rhs_expr->expr)->type) ) {
		syntax_error("invalid operator in binary expression", line, EBAD_OP);
	}

	AssignmentStmt* assignmentstmt = create_assignmentstmt(
		create_var(lvalue_token->lexeme, 0),
		rhs_expr
	);

	consume_token(NEWLINE, true);
	vector_add(stmts, create_stmt(line, ASSIGNMENT_STMT, assignmentstmt));
}

static void write_stmt(int line) {
	Expr* write_expr = expr();
	if (write_expr->type == BINARY) {
		syntax_error("invalid expression in write statement", line, EBAD_EXPR);
	}

	consume_token(NEWLINE, true);

	WriteStmt* writestmt = create_writestmt(write_expr);
	vector_add(stmts, create_stmt(line, WRITE_STMT, writestmt));
}

static void writeln_stmt(int line) {
	Expr* writeln_expr = expr();
	if (writeln_expr->type == BINARY) {
		syntax_error("invalid expression in writeln statement", line, EBAD_EXPR);
	}

	consume_token(NEWLINE, true);

	WritelnStmt* writelnstmt = create_writelnstmt(writeln_expr);
	vector_add(stmts, create_stmt(line, WRITELN_STMT, writelnstmt));
}

static void while_stmt(int line, int indent) {
	Expr* cond = expr();
	if (cond->type != BINARY ||
		  !is_comp_operator(((Binary*) cond->expr)->type) ) {
		syntax_error("invalid conditional in while statement", line, EBAD_COND);
	}

	consume_token(NEWLINE, false);

	WhileStmt* whilestmt = create_whilestmt(cond, block_stmt(line, indent));
	vector_add(stmts, create_stmt(line, WHILE_STMT, whilestmt));
}

static void if_else_stmt(int line, int indent) {
	Expr* cond = expr();
	if (cond->type != BINARY ||
		  !is_comp_operator(((Binary*) cond->expr)->type) ) {
		syntax_error("invalid conditional in if-else statement", line, EBAD_COND);
	}

	consume_token(NEWLINE, false);

	Vector then_stmts = block_stmt(line, indent);
	Vector else_stmts = NULL;

	int temp_curr_token = curr_token;
	int next_indent = compute_indentation();

	if (next_indent != indent) { // Next statement can only match with an outer block
		// Fix the stream index to read the current statement in the proper context
		curr_token = temp_curr_token;

		IfElseStmt* ifelsestmt = create_ifelsestmt(cond, then_stmts, else_stmts);
		vector_add(stmts, create_stmt(line, IF_ELSE_STMT, ifelsestmt));
		return; // End of if statement
	}

	if (match_token(ELSE)) {
		int else_line = consume_token(NEWLINE, false)->line;
		else_stmts = block_stmt(else_line, indent);
	} else {
		// Fix the stream index to read the current statement in the proper context
		curr_token = temp_curr_token;
	}

	IfElseStmt* ifelsestmt = create_ifelsestmt(cond, then_stmts, else_stmts);
	vector_add(stmts, create_stmt(line, IF_ELSE_STMT, ifelsestmt));
}

static Vector block_stmt(int line, int indent) {
	int temp_curr_indent = curr_indent;
	curr_indent = indent + 1;

	// We rely on the program's runtime stack to parse a block recursively and
	// make a new vector containing the statements it contains

	Vector curr_stmts = stmts;
	Vector block_stmts = parse(token_stream);

	// Get the state to where it was before parse()
	stmts = curr_stmts;
	curr_indent = temp_curr_indent;

	return_from_block = false;

	if (vector_size(block_stmts) == 0) {
		syntax_error("empty body statement", line, ENO_BODY);
	}
	return block_stmts;
}

static void random_stmt(int line) {
	Token* var_token = consume_token(IDENTIFIER, false);
	consume_token(NEWLINE, true);

	RandomStmt* randomstmt = create_randomstmt(create_var(var_token->lexeme, 0));
	vector_add(stmts, create_stmt(line, RANDOM_STMT, randomstmt));
}

static void arg_size_stmt(int line) {
	Token* var_token = consume_token(IDENTIFIER, false);
	consume_token(NEWLINE, true);

	ArgSizeStmt* argsizestmt = create_argsizestmt(create_var(var_token->lexeme, 0));
	vector_add(stmts, create_stmt(line, ARG_SIZE_STMT, argsizestmt));
}

static void arg_stmt(int line) {
	Expr* index_expr = expr();
	if (index_expr->type == BINARY) {
		syntax_error("invalid index in argument statement", line, EBAD_IDX);
	}

	Token* var_token = consume_token(IDENTIFIER, false);
	consume_token(NEWLINE, true);

	ArgStmt* argstmt = create_argstmt(index_expr, create_var(var_token->lexeme, 0));
	vector_add(stmts, create_stmt(line, ARG_STMT, argstmt));
}

static void break_stmt(int line) {
	consume_token(NEWLINE, true);
	vector_add(stmts, create_stmt(line, BREAK_STMT, NULL));
}

static void continue_stmt(int line) {
	consume_token(NEWLINE, true);
	vector_add(stmts, create_stmt(line, CONTINUE_STMT, NULL));
}

static Expr* expr(void) {
	Token* left = advance();
	Token* op = peek();

	ExprType left_type, right_type;
	void* left_expr;
	void* right_expr;

	var_or_literal(left, &left_expr, &left_type);

	if (!is_operator(op->type)) {
		return create_expr(left_type, left_expr);
	}

	advance(); // We already have the current token in op

	Token* right = advance();
	var_or_literal(right, &right_expr, &right_type);

	return create_expr(
		BINARY,
		create_binary(
			op->type,
			create_expr(left_type, left_expr),
			create_expr(right_type, right_expr)
		)
	);
}

static void var_or_literal(Token* token, void** expr, ExprType* type) {
	if (token->type == ENDOFFILE) {
		int err_line = ((Token*) vector_get(stmts, curr_token-2))->line;
		syntax_error("unexpected program termination", err_line, EBAD_TERM);
	}

	if (token->type == NUMBER) {
		*type = LITERAL;
		*expr = create_literal(token->literal);
	} else if (token->type == IDENTIFIER) {
		*type = VAR;
		*expr = create_var(token->lexeme, 0);
	} else {
		syntax_error("expected variable or literal", token->line, EBAD_EXPR);
	}
}

static Token* advance(void) {
	Token* token = vector_get(token_stream, curr_token);
	if (!reached_end()) curr_token++;
	return token;
}

static Token* peek(void) {
	return vector_get(token_stream, curr_token);
}

static Token* previous(void) {
	assert(curr_token > 0);
	return vector_get(token_stream, curr_token-1);
}

static Token* consume_token(TokenType type, bool endable) {
	Token* next = advance();

	if (next->type == ENDOFFILE) {
		if (endable) {
			return next;
		} else {
			syntax_error("unexpected program termination", next->line, EBAD_TERM);
		}
	} else if (next->type != type) {
		syntax_error("unexpected token", next->line, EBAD_TOK);
	}

	return next;
}

static bool match_token(TokenType type) {
	Token* next = peek();

	if (next->type != type) {
		return false;
	} else {
		curr_token++;
		return true;
	}
}

static int compute_indentation(void) {
	int indent = 0;

	while (match_token(TAB) && !reached_end()) {
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
	return ((Token*) vector_get(token_stream, curr_token))->type == ENDOFFILE;
}

static void syntax_error(char* msg, int line, int status) {
	fprintf(stderr, "Syntax Error: %s at line %d", msg, line);
	exit(status);
}
