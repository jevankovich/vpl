#ifndef PARSE_H
#define PARSE_H
#include "scan.h"

struct vector;
struct operand;
struct expr;
struct statement;

struct value {
	size_t dims;
	size_t *size;
	double *data;
};

struct value parse(struct lex_items);
#endif
