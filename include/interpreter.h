#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "vector.h"

// Executes a program that's represented as a vector of statements
void execute(Vector stmts, int argc, char **argv);

#endif // INTERPRETER_H
