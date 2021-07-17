#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

#include "vector.h"

#define MAX_LEXEME 100

// Tokenize input coming from stream and return a vector of tokens
Vector scan_tokens(FILE* stream);

#endif // SCANNER_H
