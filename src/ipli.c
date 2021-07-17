#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

#include "error.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr, "Usage: ./ipli <file> [<args>]\n");
		return EBAD_ARGS;
	}

	FILE* stream = fopen(argv[1], "r");
	if (stream == NULL) {
		fprintf(stderr, "Error: unable to open input file\n");
		return EOPEN_FILE;
	}

	srand(time(NULL));

	Vector tokens = scan_tokens(stream);
	Vector stmts = parse(tokens);

	execute(stmts, argc, argv);

	vector_destroy(tokens);
	vector_destroy(stmts);

	fclose(stream);
	return 0;
}
