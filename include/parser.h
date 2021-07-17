#ifndef PARSER_H
#define PARSER_H

#include "vector.h"

// Parses a stream of tokens against IPL's grammar and returns a vector
// of statements ready to be executed by the interpreter
Vector parse(Vector tokens);

#endif // PARSER_H
