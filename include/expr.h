#ifndef EXPR_H
#define EXPR_H

#include "token.h"

typedef enum expr_type {
	LITERAL, VAR, BINARY
} ExprType;

typedef struct expr {
	ExprType type;
	void* expr;
} Expr;

typedef struct literal {
	int value;
} Literal;

typedef struct var {
	char* id;
	int value;
} Var;

typedef struct binary {
	TokenType type;
	Expr* left;
	Expr* right;
} Binary;

// Constructors for the above types
Expr* create_expr(ExprType type, void* expr);
Literal* create_literal(int value);
Var* create_var(char* id, int value);
Binary* create_binary(TokenType type, Expr* left, Expr* right);

// Destructors for the above types
void destroy_expr(void* expr);
void destroy_literal(void* expr);
void destroy_var(void* expr);
void destroy_binary(void* expr);

#endif // EXPR_H
