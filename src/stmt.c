#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "expr.h"
#include "stmt.h"

Stmt* create_stmt(int line, StmtType type, void* stmt) {
	Stmt* new_stmt = malloc(sizeof(Stmt));
	assert(new_stmt != NULL);

	new_stmt->line = line;
	new_stmt->type = type;
	new_stmt->stmt = stmt;

	return new_stmt;
}

ReadStmt* create_read_stmt(bool is_array, void* lvalue) {
	ReadStmt* new_stmt = malloc(sizeof(ReadStmt));
	assert(new_stmt != NULL);

	new_stmt->is_array = is_array;
	new_stmt->lvalue = lvalue;

	return new_stmt;
}

AssignmentStmt* create_assignment_stmt(bool is_array, void* lvalue, Expr* expr) {
	AssignmentStmt* new_stmt = malloc(sizeof(AssignmentStmt));
	assert(new_stmt != NULL);

	new_stmt->is_array = is_array;
	new_stmt->lvalue = lvalue;
	new_stmt->expr = expr;

	return new_stmt;
}

WriteStmt* create_write_stmt(Expr* expr) {
	WriteStmt* new_stmt = malloc(sizeof(WriteStmt));
	assert(new_stmt != NULL);

	new_stmt->expr = expr;

	return new_stmt;
}

WritelnStmt* create_writeln_stmt(Expr* expr) {
	WritelnStmt* new_stmt = malloc(sizeof(WritelnStmt));
	assert(new_stmt != NULL);

	new_stmt->expr = expr;

	return new_stmt;
}

WhileStmt* create_while_stmt(Expr* cond, Vector stmts) {
	WhileStmt* new_stmt = malloc(sizeof(WhileStmt));
	assert(new_stmt != NULL);

	new_stmt->cond = cond;
	new_stmt->stmts = stmts;

	return new_stmt;
}

IfElseStmt* create_if_else_stmt(Expr* cond, Vector then_stmts, Vector else_stmts) {
	IfElseStmt* new_stmt = malloc(sizeof(IfElseStmt));
	assert(new_stmt != NULL);

	new_stmt->cond = cond;
	new_stmt->then_stmts = then_stmts;
	new_stmt->else_stmts = else_stmts;

	return new_stmt;
}

RandomStmt* create_random_stmt(bool is_array, void* lvalue) {
	RandomStmt* new_stmt = malloc(sizeof(RandomStmt));
	assert(new_stmt != NULL);

	new_stmt->is_array = is_array;
	new_stmt->lvalue = lvalue;

	return new_stmt;
}

ArgSizeStmt* create_arg_size_stmt(bool is_array, void* lvalue) {
	ArgSizeStmt* new_stmt = malloc(sizeof(ArgSizeStmt));
	assert(new_stmt != NULL);

	new_stmt->is_array = is_array;
	new_stmt->lvalue = lvalue;

	return new_stmt;
}

ArgStmt* create_arg_stmt(Expr* expr, bool is_array, void* lvalue) {
	ArgStmt* new_stmt = malloc(sizeof(ArgStmt));
	assert(new_stmt != NULL);

	new_stmt->expr = expr;
	new_stmt->is_array = is_array;
	new_stmt->lvalue = lvalue;

	return new_stmt;
}

BreakStmt* create_break_stmt(int n_loops) {
	BreakStmt* new_stmt = malloc(sizeof(BreakStmt));
	assert(new_stmt != NULL);

	new_stmt->n_loops = n_loops;

	return new_stmt;
}

ContinueStmt* create_continue_stmt(int n_loops) {
	ContinueStmt* new_stmt = malloc(sizeof(ContinueStmt));
	assert(new_stmt != NULL);

	new_stmt->n_loops = n_loops;

	return new_stmt;
}

NewStmt* create_new_stmt(char* id, Expr* size) {
	NewStmt* new_stmt = malloc(sizeof(NewStmt));
	assert(new_stmt != NULL);

	new_stmt->id = strdup(id);
	new_stmt->size = size;

	return new_stmt;
}

FreeStmt* create_free_stmt(char* id) {
	FreeStmt* new_stmt = malloc(sizeof(FreeStmt));
	assert(new_stmt != NULL);

	new_stmt->id = strdup(id);

	return new_stmt;
}

SizeStmt* create_size_stmt(char* id, bool is_array, void* lvalue) {
	SizeStmt* new_stmt = malloc(sizeof(SizeStmt));
	assert(new_stmt != NULL);

	new_stmt->id = strdup(id);
	new_stmt->is_array = is_array;
	new_stmt->lvalue = lvalue;

	return new_stmt;
}

void destroy_stmt(void* stmt) {
	assert(stmt != NULL);

	Stmt* stmtt = (Stmt*) stmt;
	switch (stmtt->type) {
		case READ_STMT: destroy_read_stmt(stmtt->stmt); break;
		case ASSIGNMENT_STMT: destroy_assignment_stmt(stmtt->stmt); break;
		case WRITE_STMT: destroy_write_stmt(stmtt->stmt); break;
		case WRITELN_STMT: destroy_writeln_stmt(stmtt->stmt); break;
		case WHILE_STMT: destroy_while_stmt(stmtt->stmt); break;
		case IF_ELSE_STMT: destroy_if_else_stmt(stmtt->stmt); break;
		case RANDOM_STMT: destroy_random_stmt(stmtt->stmt); break;
		case ARG_STMT: destroy_arg_stmt(stmtt->stmt); break;
		case ARG_SIZE_STMT: destroy_arg_size_stmt(stmtt->stmt); break;
		case NEW_STMT: destroy_new_stmt(stmtt->stmt); break;
		case FREE_STMT: destroy_free_stmt(stmtt->stmt); break;
		case SIZE_STMT: destroy_size_stmt(stmtt->stmt); break;
		case BREAK_STMT: break;
		case CONTINUE_STMT: break;
		default:
			fprintf(stderr, "Invalid statement type (this shouldn't be printed)\n");
			exit(EXIT_FAILURE);
	}

	free(stmtt);
}

void destroy_read_stmt(void* stmt) {
	assert(stmt != NULL);

	ReadStmt* stmtt = (ReadStmt*) stmt;
	if (stmtt->is_array) {
		destroy_array(stmtt->lvalue);
	} else {
		destroy_var(stmtt->lvalue);
	}

	free(stmtt);
}

void destroy_assignment_stmt(void* stmt) {
	assert(stmt != NULL);

	AssignmentStmt* stmtt = (AssignmentStmt*) stmt;
	if (stmtt->is_array) {
		destroy_array(stmtt->lvalue);
	} else {
		destroy_var(stmtt->lvalue);
	}

	destroy_expr(stmtt->expr);
	free(stmtt);
}

void destroy_write_stmt(void* stmt) {
	assert(stmt != NULL);

	WriteStmt* stmtt = (WriteStmt*) stmt;
	if (stmtt->expr != NULL) {
		destroy_expr(stmtt->expr);
	}

	free(stmtt);
}

void destroy_writeln_stmt(void* stmt) {
	assert(stmt != NULL);

	WritelnStmt* stmtt = (WritelnStmt*) stmt;
	if (stmtt->expr != NULL) {
		destroy_expr(stmtt->expr);
	}

	free(stmtt);
}

void destroy_while_stmt(void* stmt) {
	assert(stmt != NULL);

	WhileStmt* stmtt = (WhileStmt*) stmt;
	destroy_expr(stmtt->cond);
	vector_destroy(stmtt->stmts);
	free(stmtt);
}

void destroy_if_else_stmt(void* stmt) {
	assert(stmt != NULL);

	IfElseStmt* stmtt = (IfElseStmt*) stmt;
	destroy_expr(stmtt->cond);
	vector_destroy(stmtt->then_stmts);
	
	if (stmtt->else_stmts != NULL) {
		vector_destroy(stmtt->else_stmts);
	}

	free(stmtt);
}

void destroy_random_stmt(void* stmt) {
	assert(stmt != NULL);

	RandomStmt* stmtt = (RandomStmt*) stmt;
	if (stmtt->is_array) {
		destroy_array(stmtt->lvalue);
	} else {
		destroy_var(stmtt->lvalue);
	}

	free(stmtt);
}

void destroy_arg_size_stmt(void* stmt) {
	assert(stmt != NULL);

	ArgSizeStmt* stmtt = (ArgSizeStmt*) stmt;
	if (stmtt->is_array) {
		destroy_array(stmtt->lvalue);
	} else {
		destroy_var(stmtt->lvalue);
	}

	free(stmtt);
}

void destroy_arg_stmt(void* stmt) {
	assert(stmt != NULL);

	ArgStmt* stmtt = (ArgStmt*) stmt;
	destroy_expr(stmtt->expr);

	if (stmtt->is_array) {
		destroy_array(stmtt->lvalue);
	} else {
		destroy_var(stmtt->lvalue);
	}

	free(stmtt);
}

void destroy_break_stmt(void* stmt) {
	assert(stmt != NULL);

	BreakStmt* stmtt = (BreakStmt*) stmt;
	free(stmtt);
}

void destroy_continue_stmt(void* stmt) {
	assert(stmt != NULL);

	ContinueStmt* stmtt = (ContinueStmt*) stmt;
	free(stmtt);
}

void destroy_new_stmt(void* stmt) {
	assert(stmt != NULL);

	NewStmt* stmtt = (NewStmt*) stmt;
	free(stmtt->id);
	destroy_expr(stmtt->size);
	free(stmtt);
}

void destroy_free_stmt(void* stmt) {
	assert(stmt != NULL);

	FreeStmt* stmtt = (FreeStmt*) stmt;
	free(stmtt->id);
	free(stmtt);
}

void destroy_size_stmt(void* stmt) {
	assert(stmt != NULL);

	SizeStmt* stmtt = (SizeStmt*) stmt;
	if (stmtt->is_array) {
		destroy_array(stmtt->lvalue);
	} else {
		destroy_var(stmtt->lvalue);
	}

	free(stmtt->id);
	free(stmtt);
}
