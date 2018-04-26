#ifndef PARSE_H
#define PARSE_H
#include "scan.h"

struct expr;

// Statement
//   identifier '=' Expr
//   Expr
struct statement {
	int error; // If !=0, var contains the error string
	char *var;
	struct expr *expr;
};

struct statement *parse(struct lex_items);
#endif
