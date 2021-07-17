#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "stmt.h"

Stmt* create_stmt(int line, StmtType type, void* stmt) {
	Stmt* new_stmt = malloc(sizeof(Stmt));
	assert(new_stmt != NULL);

	new_stmt->line = line;
	new_stmt->type = type;
	new_stmt->stmt = stmt;

	return new_stmt;
}

ReadStmt* create_readstmt(Var* var) {
	ReadStmt* new_stmt = malloc(sizeof(ReadStmt));
	assert(new_stmt != NULL);

	new_stmt->var = var;

	return new_stmt;
}

AssignmentStmt* create_assignmentstmt(Var* lvalue, Expr* expr) {
	AssignmentStmt* new_stmt = malloc(sizeof(AssignmentStmt));
	assert(new_stmt != NULL);

	new_stmt->lvalue = lvalue;
	new_stmt->expr = expr;

	return new_stmt;
}

WriteStmt* create_writestmt(Expr* expr) {
	WriteStmt* new_stmt = malloc(sizeof(WriteStmt));
	assert(new_stmt != NULL);

	new_stmt->expr = expr;

	return new_stmt;
}

WritelnStmt* create_writelnstmt(Expr* expr) {
	WritelnStmt* new_stmt = malloc(sizeof(WritelnStmt));
	assert(new_stmt != NULL);

	new_stmt->expr = expr;

	return new_stmt;
}

WhileStmt* create_whilestmt(Expr* cond, Vector stmts) {
	WhileStmt* new_stmt = malloc(sizeof(WhileStmt));
	assert(new_stmt != NULL);

	new_stmt->cond = cond;
	new_stmt->stmts = stmts;

	return new_stmt;
}

IfElseStmt* create_ifelsestmt(Expr* cond, Vector then_stmts, Vector else_stmts) {
	IfElseStmt* new_stmt = malloc(sizeof(IfElseStmt));
	assert(new_stmt != NULL);

	new_stmt->cond = cond;
	new_stmt->then_stmts = then_stmts;
	new_stmt->else_stmts = else_stmts;

	return new_stmt;
}

RandomStmt* create_randomstmt(Var* var) {
	RandomStmt* new_stmt = malloc(sizeof(RandomStmt));
	assert(new_stmt != NULL);

	new_stmt->var = var;

	return new_stmt;
}

ArgStmt* create_argstmt(Expr* expr, Var* var) {
	ArgStmt* new_stmt = malloc(sizeof(ArgStmt));
	assert(new_stmt != NULL);

	new_stmt->expr = expr;
	new_stmt->var = var;

	return new_stmt;
}

ArgSizeStmt* create_argsizestmt(Var* var) {
	ArgSizeStmt* new_stmt = malloc(sizeof(ArgSizeStmt));
	assert(new_stmt != NULL);

	new_stmt->var = var;

	return new_stmt;
}

void destroy_stmt(void* stmt) {
	assert(stmt != NULL);

	Stmt* stmtt = (Stmt*) stmt;
	switch (stmtt->type) {
		case READ_STMT: destroy_readstmt(stmtt->stmt); break;
		case ASSIGNMENT_STMT: destroy_assignmentstmt(stmtt->stmt); break;
		case WRITE_STMT: destroy_writestmt(stmtt->stmt); break;
		case WRITELN_STMT: destroy_writelnstmt(stmtt->stmt); break;
		case WHILE_STMT: destroy_whilestmt(stmtt->stmt); break;
		case IF_ELSE_STMT: destroy_ifelsestmt(stmtt->stmt); break;
		case RANDOM_STMT: destroy_randomstmt(stmtt->stmt); break;
		case ARG_STMT: destroy_argstmt(stmtt->stmt); break;
		case ARG_SIZE_STMT: destroy_argsizestmt(stmtt->stmt); break;
		case BREAK_STMT:
		case CONTINUE_STMT:
			break;
		default:
			fprintf(stderr, "Invalid statement type (this shouldn't be printed)\n");
			exit(EXIT_FAILURE);
	}

	free(stmtt);
}

void destroy_readstmt(void* stmt) {
	assert(stmt != NULL);

	ReadStmt* stmtt = (ReadStmt*) stmt;
	destroy_var(stmtt->var);
	free(stmtt);
}

void destroy_assignmentstmt(void* stmt) {
	assert(stmt != NULL);

	AssignmentStmt* stmtt = (AssignmentStmt*) stmt;
	destroy_var(stmtt->lvalue);
	destroy_expr(stmtt->expr);
	free(stmtt);
}

void destroy_writestmt(void* stmt) {
	assert(stmt != NULL);

	WriteStmt* stmtt = (WriteStmt*) stmt;
	destroy_expr(stmtt->expr);
	free(stmtt);
}

void destroy_writelnstmt(void* stmt) {
	assert(stmt != NULL);

	WritelnStmt* stmtt = (WritelnStmt*) stmt;
	destroy_expr(stmtt->expr);
	free(stmtt);
}

void destroy_whilestmt(void* stmt) {
	assert(stmt != NULL);

	WhileStmt* stmtt = (WhileStmt*) stmt;
	destroy_expr(stmtt->cond);
	vector_destroy(stmtt->stmts);
	free(stmtt);
}

void destroy_ifelsestmt(void* stmt) {
	assert(stmt != NULL);

	IfElseStmt* stmtt = (IfElseStmt*) stmt;
	destroy_expr(stmtt->cond);
	vector_destroy(stmtt->then_stmts);
	
	if (stmtt->else_stmts != NULL) {
		vector_destroy(stmtt->else_stmts);
	}

	free(stmtt);
}

void destroy_randomstmt(void* stmt) {
	assert(stmt != NULL);

	RandomStmt* stmtt = (RandomStmt*) stmt;
	destroy_var(stmtt->var);
	free(stmtt);
}

void destroy_argstmt(void* stmt) {
	assert(stmt != NULL);

	ArgStmt* stmtt = (ArgStmt*) stmt;
	destroy_expr(stmtt->expr);
	destroy_var(stmtt->var);
	free(stmtt);
}

void destroy_argsizestmt(void* stmt) {
	assert(stmt != NULL);

	ArgSizeStmt* stmtt = (ArgSizeStmt*) stmt;
	destroy_var(stmtt->var);
	free(stmtt);
}
