#ifndef STMT_H
#define STMT_H

#include <stdbool.h>

#include "expr.h"
#include "vector.h"

typedef enum stmt_type {
	READ_STMT, ASSIGNMENT_STMT,
	WRITE_STMT, WRITELN_STMT,
	WHILE_STMT, IF_ELSE_STMT,
	RANDOM_STMT, ARG_STMT, ARG_SIZE_STMT,
	BREAK_STMT, CONTINUE_STMT,
	NEW_STMT, FREE_STMT
} StmtType;

typedef struct stmt {
	int line; // This comes in handy for error handling
	StmtType type;
	void* stmt;
} Stmt;

typedef struct read_stmt {
	bool is_array;
	void* lvalue;
} ReadStmt;

typedef struct assignment_stmt {
	bool is_array;
	void* lvalue;
	Expr* expr;
} AssignmentStmt;

typedef struct write_stmt {
	Expr* expr;
} WriteStmt;

typedef struct writeln_stmt {
	Expr* expr;
} WritelnStmt;

typedef struct while_stmt {
	Expr* cond;
	Vector stmts;
} WhileStmt;

typedef struct if_else_stmt {
	Expr* cond;
	Vector then_stmts;
	Vector else_stmts;
} IfElseStmt;

typedef struct random_stmt {
	bool is_array;
	void* lvalue;
} RandomStmt;

typedef struct arg_size_stmt {
	bool is_array;
	void* lvalue;
} ArgSizeStmt;

typedef struct arg_stmt {
	Expr* expr;
	bool is_array;
	void* lvalue;
} ArgStmt;

typedef struct break_stmt {
	int n_loops;
} BreakStmt;

typedef struct continue_stmt {
	int n_loops;
} ContinueStmt;

typedef struct new_stmt {
	char* id;
	Expr* size;
} NewStmt;

typedef struct free_stmt {
	char* id;
} FreeStmt;

// Constructors for the above types
Stmt* create_stmt(int line, StmtType type, void* stmt);
ReadStmt* create_read_stmt(bool is_array, void* lvalue);
AssignmentStmt* create_assignment_stmt(bool is_array, void* lvalue, Expr* expr);
WriteStmt* create_write_stmt(Expr* expr);
WritelnStmt* create_writeln_stmt(Expr* expr);
WhileStmt* create_while_stmt(Expr* cond, Vector stmts);
IfElseStmt* create_if_else_stmt(Expr* cond, Vector then_stmts, Vector else_stmts);
RandomStmt* create_random_stmt(bool is_array, void* lvalue);
ArgSizeStmt* create_arg_size_stmt(bool is_array, void* lvalue);
ArgStmt* create_arg_stmt(Expr* expr, bool is_array, void* lvalue);
BreakStmt* create_break_stmt(int n_loops);
ContinueStmt* create_continue_stmt(int n_loops);
NewStmt* create_new_stmt(char* id, Expr* size);
FreeStmt* create_free_stmt(char* id);

// Destructors for the above types
void destroy_stmt(void* stmt);
void destroy_read_stmt(void* stmt);
void destroy_assignment_stmt(void* stmt);
void destroy_write_stmt(void* stmt);
void destroy_writeln_stmt(void* stmt);
void destroy_while_stmt(void* stmt);
void destroy_if_else_stmt(void* stmt);
void destroy_random_stmt(void* stmt);
void destroy_arg_size_stmt(void* stmt);
void destroy_arg_stmt(void* stmt);
void destroy_break_stmt(void* stmt);
void destroy_continue_stmt(void* stmt);
void destroy_new_stmt(void* stmt);
void destroy_free_stmt(void* stmt);

#endif // STMT_H
