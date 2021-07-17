#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "map.h"
#include "vector.h"

#include "stmt.h"
#include "expr.h"
#include "error.h"
#include "interpreter.h"

// Helper functions used by the interpreter (no reason to expose them)
static void execute_read_stmt(int line, ReadStmt* stmt);
static void execute_assignment_stmt(int line, AssignmentStmt* stmt);
static void execute_write_stmt(int line, WriteStmt* stmt);
static void execute_writeln_stmt(int line, WritelnStmt* stmt);
static void execute_while_stmt(int line, WhileStmt* stmt);
static void execute_if_else_stmt(int line, IfElseStmt* stmt);
static void execute_random_stmt(int line, RandomStmt* stmt);
static void execute_arg_stmt(int line, ArgStmt* stmt);
static void execute_arg_size_stmt(int line, ArgSizeStmt* stmt);
static void execute_break_stmt(int line);
static void execute_continue_stmt(int line);
static int evaluate_expr(int line, Expr* expr);
static int evaluate_literal(int line, Literal* expr);
static int evaluate_var(int line, Var* expr);
static int evaluate_binary(int line, Binary* expr);
static void runtime_error(char* msg, int line, int status);

// Interpreter state is maintained with the help of the following globals
static Map symbol_table; // Implements the mapping identifier -> value
static int nesting = 0;

static int n_args;
static char** args;

static enum { STOP, REPEAT, NORMAL } loop_state = NORMAL;
static bool inside_while_block = false;

void execute(Vector stmts, int argc, char **argv) {
	static bool initialized = false;

	if (!initialized) {
		n_args = argc;
		args = argv;
		symbol_table = map_create(NULL, NULL, NULL, NULL);
		initialized = true;
	}

	int n_statements = vector_size(stmts);
	for (int i = 0; i < n_statements; i++) {
		Stmt* stmt = vector_get(stmts, i);

		switch (stmt->type) {
			case READ_STMT: execute_read_stmt(stmt->line, stmt->stmt); break;
			case ASSIGNMENT_STMT: execute_assignment_stmt(stmt->line, stmt->stmt); break;
			case WRITE_STMT: execute_write_stmt(stmt->line, stmt->stmt); break;
			case WRITELN_STMT: execute_writeln_stmt(stmt->line, stmt->stmt); break;
			case WHILE_STMT: execute_while_stmt(stmt->line, stmt->stmt); break;
			case IF_ELSE_STMT: execute_if_else_stmt(stmt->line, stmt->stmt); break;
			case RANDOM_STMT: execute_random_stmt(stmt->line, stmt->stmt); break;
			case ARG_STMT: execute_arg_stmt(stmt->line, stmt->stmt); break;
			case ARG_SIZE_STMT: execute_arg_size_stmt(stmt->line, stmt->stmt); break;
			case BREAK_STMT: execute_break_stmt(stmt->line); break;
			case CONTINUE_STMT: execute_continue_stmt(stmt->line); break;
			default:
				fprintf(stderr, "Invalid statement type (this shouldn't be printed)\n");
				exit(EXIT_FAILURE);
		}

		if (loop_state != NORMAL) {
			return; // A break or continue statement was encountered
		}
	}

	if (nesting == 0) {
		// Only destroy the symbol table if we're at the top level & finished
		map_destroy(symbol_table);
	}
}

static void execute_read_stmt(int line, ReadStmt* stmt) {
	Var* var = stmt->var;
	scanf("%d", &var->value);
	map_put(symbol_table, var->id, &var->value);
}

static void execute_assignment_stmt(int line, AssignmentStmt* stmt) {
	Var* lvalue = stmt->lvalue;

	lvalue->value = evaluate_expr(line, stmt->expr);
	map_put(symbol_table, lvalue->id, &lvalue->value);
}

static void execute_write_stmt(int line, WriteStmt* stmt) {
	printf("%d", evaluate_expr(line, stmt->expr));
}

static void execute_writeln_stmt(int line, WritelnStmt* stmt) {
	printf("%d\n", evaluate_expr(line, stmt->expr));
}

static void execute_while_stmt(int line, WhileStmt* stmt) {
	bool temp = inside_while_block;

	inside_while_block = true;
	nesting++;

	while (true) {
		int cond = evaluate_expr(line, stmt->cond);
		if (cond == 0) break;

		loop_state = NORMAL;
		execute(stmt->stmts, n_args, args);

		if (loop_state == STOP) {
			break;
		}
	}

	inside_while_block = temp;
	loop_state = NORMAL;
	nesting--;
}

static void execute_if_else_stmt(int line, IfElseStmt* stmt) {
	int cond = evaluate_expr(line, stmt->cond);

	nesting++;
	if (cond == 1) {
		execute(stmt->then_stmts, n_args, args);
	} else if (stmt->else_stmts != NULL) {
		execute(stmt->else_stmts, n_args, args);
	}

	nesting--;
}

static void execute_random_stmt(int line, RandomStmt* stmt) {
	Var* var = stmt->var;
	var->value = rand();
	map_put(symbol_table, var->id, &var->value);
}

static void execute_arg_stmt(int line, ArgStmt* stmt) {
	int pos = evaluate_expr(line, stmt->expr);
	Var* var = stmt->var;

	if (pos < 1 || pos > n_args-2) {
		runtime_error("invalid argument index", line, EBAD_IDX);
	}

	var->value = atoi(args[pos+1]);
	map_put(symbol_table, var->id, &var->value);
}

static void execute_arg_size_stmt(int line, ArgSizeStmt* stmt) {
	Var* var = stmt->var;
	var->value = n_args;
	map_put(symbol_table, var->id, &var->value);
}

static void execute_break_stmt(int line) {
	if (!inside_while_block) {
		runtime_error("invalid break statement", line, EBAD_BREAK);
	}

	loop_state = STOP;
}

static void execute_continue_stmt(int line) {
	if (!inside_while_block) {
		runtime_error("invalid continue statement", line, EBAD_CONT);
	}

	loop_state = REPEAT;
}

static int evaluate_expr(int line, Expr* expr) {
	switch (expr->type) {
		case LITERAL: return evaluate_literal(line, expr->expr);
		case VAR: return evaluate_var(line, expr->expr);
		case BINARY: return evaluate_binary(line, expr->expr);
		default:
			fprintf(stderr, "Invalid expression type (this shouldn't be printed)\n");
			exit(EXIT_FAILURE);
	}
}

static int evaluate_literal(int line, Literal* expr) {
	return expr->value;
}

static int evaluate_var(int line, Var* expr) {
	int* target_value = map_get(symbol_table, expr->id);
	if (target_value == NULL) {
		// If an unseen variable is used in an expression, it's installed with value = 0
		expr->value = 0;
		map_put(symbol_table, expr->id, &expr->value);
		target_value = &expr->value;
	}

	return *target_value;
}

static int evaluate_binary(int line, Binary* expr) {
	int left = evaluate_expr(line, expr->left);
	int right = evaluate_expr(line, expr->right);

	switch (expr->type) {
		case PLUS: return left + right;
		case MINUS: return left - right;
		case STAR: return left * right;

		case SLASH:
			if (right == 0) {
				runtime_error("division with 0", line, EDIV_ZERO);
			}
			return left / right;

		case MODULO:
			if (right == 0) {
				runtime_error("division with 0", line, EDIV_ZERO);
			}
			return left % right;

    case EQUAL_EQUAL: return left == right;
    case BANG_EQUAL: return left != right;
    case LESS: return left < right;
    case LESS_EQUAL: return left <= right;
    case GREATER: return left > right;
    case GREATER_EQUAL: return left >= right;
    default:
			fprintf(stderr, "Invalid operator type (this shouldn't be printed)\n");
			exit(EXIT_FAILURE);
	}
}

static void runtime_error(char* msg, int line, int status) {
	fprintf(stderr, "Runtime Error: %s at line %d", msg, line);
	exit(status);
}
