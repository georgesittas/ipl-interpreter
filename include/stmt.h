#ifndef STMT_H
#define STMT_H

#include "expr.h"
#include "vector.h"

typedef enum stmt_type {
	READ_STMT, ASSIGNMENT_STMT,
	WRITE_STMT, WRITELN_STMT,
	WHILE_STMT, IF_ELSE_STMT,
	RANDOM_STMT, ARG_STMT, ARG_SIZE_STMT,
	BREAK_STMT, CONTINUE_STMT
} StmtType;

typedef struct stmt {
	int line; // This comes in handy for error handling
	StmtType type;
	void* stmt;
} Stmt;

typedef struct read_stmt {
	Var* var;
} ReadStmt;

typedef struct assignment_stmt {
	Var* lvalue;
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
	Var* var;
} RandomStmt;

typedef struct arg_stmt {
	Expr* expr;
	Var* var;
} ArgStmt;

typedef struct arg_size_stmt {
	Var* var;
} ArgSizeStmt;

// Constructors for the above types
Stmt* create_stmt(int line, StmtType type, void* stmt);
ReadStmt* create_readstmt(Var* var);
AssignmentStmt* create_assignmentstmt(Var* lvalue, Expr* expr);
WriteStmt* create_writestmt(Expr* expr);
WritelnStmt* create_writelnstmt(Expr* expr);
WhileStmt* create_whilestmt(Expr* cond, Vector stmts);
IfElseStmt* create_ifelsestmt(Expr* cond, Vector then_stmts, Vector else_stmts);
RandomStmt* create_randomstmt(Var* var);
ArgStmt* create_argstmt(Expr* expr, Var* var);
ArgSizeStmt* create_argsizestmt(Var* var);

// Destructors for the above types
void destroy_stmt(void* stmt);
void destroy_readstmt(void* stmt);
void destroy_assignmentstmt(void* stmt);
void destroy_writestmt(void* stmt);
void destroy_writelnstmt(void* stmt);
void destroy_whilestmt(void* stmt);
void destroy_ifelsestmt(void* stmt);
void destroy_randomstmt(void* stmt);
void destroy_argstmt(void* stmt);
void destroy_argsizestmt(void* stmt);

#endif // STMT_H
