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
static TableEntry* create_table_entry(ExprType type, void* value);
static void execute_read_stmt(int line, ReadStmt* stmt);
static void execute_assignment_stmt(int line, AssignmentStmt* stmt);
static void execute_write_stmt(int line, WriteStmt* stmt);
static void execute_writeln_stmt(int line, WritelnStmt* stmt);
static void execute_while_stmt(int line, WhileStmt* stmt);
static void execute_if_else_stmt(int line, IfElseStmt* stmt);
static void execute_random_stmt(int line, RandomStmt* stmt);
static void execute_arg_stmt(int line, ArgStmt* stmt);
static void execute_arg_size_stmt(int line, ArgSizeStmt* stmt);
static void execute_break_stmt(int line, BreakStmt* stmt);
static void execute_continue_stmt(int line, ContinueStmt* stmt);
static void execute_new_stmt(int line, NewStmt* stmt);
static void execute_free_stmt(int line, FreeStmt* stmt);
static int evaluate_expr(int line, Expr* expr);
static int evaluate_literal(int line, Literal* expr);
static int evaluate_var(int line, Var* expr);
static int evaluate_array(int line, Array* expr);
static int evaluate_binary(int line, Binary* expr);
static void assign_to_lvalue(int line, int value, bool is_array, void* lvalue);
static void runtime_error(char* msg, int line, int status);

// Interpreter state is maintained with the help of the following globals
static Map symbol_table; // Implements the mapping identifier -> value
static int nesting = 0;

static int n_args;
static char** args;

static int loop_nesting = 0;
static int jump_n_loops = 1; // Used for break <n> and continue <n>
static enum { STOP, REPEAT, NORMAL } loop_state = NORMAL;

static TableEntry* create_table_entry(ExprType type, void* value) {
	TableEntry* new_table_entry = malloc(sizeof(TableEntry));
	assert(new_table_entry != NULL);

	new_table_entry->type = type;
	new_table_entry->value = value;

	return new_table_entry;
}

void execute(Vector stmts, int argc, char **argv) {
	static bool initialized = false;

	if (!initialized) {
		n_args = argc;
		args = argv;
		symbol_table = map_create(NULL, NULL, free, NULL);
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
			case BREAK_STMT: execute_break_stmt(stmt->line, stmt->stmt); break;
			case CONTINUE_STMT: execute_continue_stmt(stmt->line, stmt->stmt); break;
			case NEW_STMT: execute_new_stmt(stmt->line, stmt->stmt); break;
			case FREE_STMT: execute_free_stmt(stmt->line, stmt->stmt); break;
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
	int input;
	scanf("%d", &input);
	assign_to_lvalue(line, input, stmt->is_array, stmt->lvalue);
}

static void execute_assignment_stmt(int line, AssignmentStmt* stmt) {
	assign_to_lvalue(line, evaluate_expr(line, stmt->expr), stmt->is_array, stmt->lvalue);
}

static void execute_write_stmt(int line, WriteStmt* stmt) {
	if (stmt->expr != NULL) {
		printf("%d", evaluate_expr(line, stmt->expr));
	}
	printf(" ");
}

static void execute_writeln_stmt(int line, WritelnStmt* stmt) {
	if (stmt->expr != NULL) {
		printf("%d", evaluate_expr(line, stmt->expr));
	}
	printf("\n");
}

static void execute_while_stmt(int line, WhileStmt* stmt) {
	nesting++;
	loop_nesting++;

	while (true) {
		int cond = evaluate_expr(line, stmt->cond);
		if (cond == 0) break;

		jump_n_loops = 1;
		loop_state = NORMAL;
		execute(stmt->stmts, n_args, args);

		if (loop_state != NORMAL) {
			jump_n_loops--;
			if (loop_state == STOP || jump_n_loops != 0) {
				break;
			}
		}
	}

	if (jump_n_loops == 0) {
		jump_n_loops = 1;
		loop_state = NORMAL;
	}

	nesting--;
	loop_nesting--;
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
	assign_to_lvalue(line, rand(), stmt->is_array, stmt->lvalue);
}

static void execute_arg_stmt(int line, ArgStmt* stmt) {
	int pos = evaluate_expr(line, stmt->expr);
	if (pos < 1 || pos > n_args-2) {
		runtime_error("invalid argument index", line, EBAD_IDX);
	}

	assign_to_lvalue(line, atoi(args[pos+1]), stmt->is_array, stmt->lvalue);
}

static void execute_arg_size_stmt(int line, ArgSizeStmt* stmt) {
	assign_to_lvalue(line, n_args, stmt->is_array, stmt->lvalue);
}

static void execute_break_stmt(int line, BreakStmt* stmt) {
	if (stmt->n_loops > loop_nesting) {
		runtime_error("invalid break statement", line, EBAD_BREAK);
	}

	loop_state = STOP;
	jump_n_loops = stmt->n_loops;
}

static void execute_continue_stmt(int line, ContinueStmt* stmt) {
	if (stmt->n_loops > loop_nesting) {
		runtime_error("invalid continue statement", line, EBAD_CONT);
	}

	loop_state = REPEAT;
	jump_n_loops = stmt->n_loops;
}

static void execute_new_stmt(int line, NewStmt* stmt) {
	TableEntry* entry = map_get(symbol_table, stmt->id);
	if (entry != NULL) {
		if (entry->type == VAR) {
			runtime_error("array name overlaps with variable name", line, EBAD_ID);
		}

		free(entry->value); // Old array gets deallocated
	}

	int size = evaluate_expr(line, stmt->size);
	if (size <= 0) {
		runtime_error("array size must be greater than 0", line, EBAD_SIZE);
	}

	int* arr = calloc(size+1, sizeof(int)); // Implicit 0-initialization
	assert(arr != NULL);

	arr[0] = size; // E.g. {1,2,3} is represented as {3, 1, 2, 3}
	map_put(symbol_table, stmt->id, create_table_entry(ARRAY, arr));
}

static void execute_free_stmt(int line, FreeStmt* stmt) {
	TableEntry* entry = map_get(symbol_table, stmt->id);
	if (entry == NULL || entry->type != ARRAY) {
		runtime_error("name does not correspond to an array", line, EBAD_ARRAY);
	}

	free(entry->value);

	// Virtual removal of entry (free(NULL) is a no-op, so we're ok with destroy_value)
	map_put(symbol_table, stmt->id, NULL);
}

static int evaluate_expr(int line, Expr* expr) {
	switch (expr->type) {
		case LITERAL: return evaluate_literal(line, expr->expr);
		case VAR: return evaluate_var(line, expr->expr);
		case ARRAY: return evaluate_array(line, expr->expr);
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
	TableEntry* entry = map_get(symbol_table, expr->id);
	if (entry == NULL) {
		// If an unseen variable is used in an expression, it's installed with value = 0
		expr->value = 0;
		entry = create_table_entry(VAR, &expr->value);
		map_put(symbol_table, expr->id, entry);
	} else if (entry->type == ARRAY) {
		runtime_error("expected a variable name", line, EBAD_VAR);
	}

	return *((int*) entry->value);
}

static int evaluate_array(int line, Array* expr) {
	TableEntry* entry = map_get(symbol_table, expr->id);
	if (entry == NULL || entry->type != ARRAY) {
		runtime_error("name does not correspond to an array", line, EBAD_ARRAY);
	}

	int idx = evaluate_expr(line, expr->index);
	if (idx < 0 || idx >= ((int*) entry->value)[0]) {
		runtime_error("array index out of bounds", line, EIDX_OOB);
	}

	return ((int*) entry->value)[idx+1];
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

static void assign_to_lvalue(int line, int value, bool is_array, void* lvalue) {
	if (is_array) {
		Array* array = (Array*) lvalue;

		TableEntry* entry = map_get(symbol_table, array->id);
		if (entry == NULL || entry->type != ARRAY) {
			runtime_error("name does not correspond to an array", line, EBAD_ARRAY);
		}

		int idx = evaluate_expr(line, array->index);
		if (idx < 0 || idx >= ((int*) entry->value)[0]) {
			runtime_error("array index out of bounds", line, EIDX_OOB);
		}

		((int*) entry->value)[idx+1] = value;
	} else {
		Var* var = (Var*) lvalue;

		TableEntry* entry = map_get(symbol_table, var->id);
		if (entry == NULL) {
			var->value = value;
			map_put(symbol_table, var->id, create_table_entry(VAR, &var->value));
		} else if (entry->type == ARRAY) {
			runtime_error("expected a variable name", line, EBAD_VAR);
		} else {
			*((int*) entry->value) = value;
		}
	}
}

static void runtime_error(char* msg, int line, int status) {
	fprintf(stderr, "Runtime Error: %s at line %d\n", msg, line);
	exit(status);
}
