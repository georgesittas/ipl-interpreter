#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "token.h"

Expr* create_expr(ExprType type, void* expr) {
	Expr* new_expr = malloc(sizeof(Expr));
	assert(new_expr != NULL);

	new_expr->type = type;
	new_expr->expr = expr;

	return new_expr;
}

Literal* create_literal(int value) {
	Literal* new_literal = malloc(sizeof(Literal));
	assert(new_literal != NULL);

	new_literal->value = value;

	return new_literal;
}

Var* create_var(char* id, int value) {
	Var* new_var = malloc(sizeof(Var));
	assert(new_var != NULL);

	new_var->id = strdup(id);
	new_var->value = value;

	return new_var;
}

Array* create_array(char* id, int size) {
	Array* new_array = malloc(sizeof(Array));
	assert(new_array != NULL);

	new_array->id = strdup(id);
	new_array->size = size;

	new_array->values = calloc(size, sizeof(int));
	assert(new_array->values != NULL);

	return new_array;
}

Binary* create_binary(TokenType type, Expr* left, Expr* right) {
	Binary* new_binary = malloc(sizeof(Binary));
	assert(new_binary != NULL);

	new_binary->type = type;
	new_binary->left = left;
	new_binary->right = right;

	return new_binary;
}

void destroy_expr(void* expr) {
	assert(expr != NULL);

	Expr* exprr = (Expr*) expr;
	switch (exprr->type) {
		case LITERAL: destroy_literal(exprr->expr); break;
		case VAR: destroy_var(exprr->expr); break;
		case BINARY: destroy_binary(exprr->expr); break;
		default:
			fprintf(stderr, "Invalid expression type (shouldn't be printed)\n");
			exit(EXIT_FAILURE);
	}

	free(exprr);
}

void destroy_literal(void* expr) {
	assert(expr != NULL);

	Literal* exprr = (Literal*) expr;
	free(exprr);
}

void destroy_var(void* expr) {
	assert(expr != NULL);

	Var* exprr = (Var*) expr;
	free(exprr->id);
	free(exprr);
}

void destroy_array(void* expr) {
	assert(expr != NULL);

	Array* exprr = (Array*) expr;
	free(exprr->id);
	free(exprr->values);
	free(exprr);
}

void destroy_binary(void* expr) {
	assert(expr != NULL);

	Binary* exprr = (Binary*) expr;
	destroy_expr(exprr->left);
	destroy_expr(exprr->right);
	free(exprr);
}
