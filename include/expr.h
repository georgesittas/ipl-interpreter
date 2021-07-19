#ifndef EXPR_H
#define EXPR_H

#include "token.h"

typedef enum expr_type {
	LITERAL, VAR, ARRAY, BINARY
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
	int value; // This will be useful for the runtime
} Var;

typedef struct array {
	char* id;
	Expr* index;
} Array;

typedef struct binary {
	TokenType type;
	Expr* left;
	Expr* right;
} Binary;

// Constructors for the above types
Expr* create_expr(ExprType type, void* expr);
Literal* create_literal(int value);
Var* create_var(char* id);
Array* create_array(char* id, Expr* index);
Binary* create_binary(TokenType type, Expr* left, Expr* right);

// Destructors for the above types
void destroy_expr(void* expr);
void destroy_literal(void* expr);
void destroy_var(void* expr);
void destroy_array(void* expr);
void destroy_binary(void* expr);

#endif // EXPR_H
