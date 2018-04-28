#include "parse.h"
#include "scan.h"

#include <stdlib.h>
#include <stdio.h>

struct parser;

static struct vector *vector(struct parser *);
static struct operand *operand(struct parser *);
static struct expr *expr(struct parser *);
static struct statement statement(struct parser *);

struct parser {
	int error;
	char *errorstring;

	size_t pos;
	struct lex_items toks;
};

static struct lex_item peek(struct parser *p) {
	return p->toks.items[p->pos];
}

static struct lex_item next(struct parser *p) {
	return p->toks.items[p->pos++];
}

static void backup(struct parser *p) {
	p->pos--;
}

// number
// identifier
// symbol

// Unop
//  symbol
//  identifier

// Binop
//   symbol
//   identifier

static char *op(struct parser *p) {
	printf("op(<%s>...)\n", peek(p).value);
	switch (peek(p).type) {
		case TOKEN_ERROR:
			p->error = 1;
			p->errorstring = next(p).value;
			return NULL;
		case TOKEN_SYM:
		case TOKEN_IDENT:
			return next(p).value;
		default:
			p->error = 1;
			p->errorstring = "Expected Identifier or Symbol";
			return NULL;
	}
}

// Vector
//   number vector
//   number
struct vector {
	char *num;
	struct vector *next;
};

static struct vector *vector(struct parser *p) {
	printf("vector(<%s>...)\n", peek(p).value);
	if (peek(p).type != TOKEN_NUMBER) {
		p->error = 1;
		p->errorstring = "Expected a number";
		return NULL;
	}

	struct vector *ret = malloc(sizeof(*ret));
	ret->num = next(p).value;
	
	if (peek(p).type == TOKEN_NUMBER) {
		ret->next = vector(p);
	} else {
		ret->next = NULL;
	}

	return ret;
}

// Operand
//   ( Expr )
//   Vector
//   identifier
//   Operand [ Expr ]
enum operand_type {
	OP_VECTOR,
	OP_IDENT,
	OP_PAREN
};

struct operand {
	enum operand_type type;
	union {
		struct expr *expr; // ( Expr )
		struct vector *vector; // Vector
		char *ident; // identifier
	};

	int hasindex;
	struct expr *index; // Operand [ Expr ]
};

static struct operand *operand(struct parser *p) {
	printf("operand(<%s>...)\n", peek(p).value);
	struct operand *ret = malloc(sizeof(*ret));

	struct lex_item tok = peek(p);
	if (tok.type == TOKEN_LPAR) { // ( Expr )
		ret->type = OP_PAREN;
		next(p);
		ret->expr = expr(p);
		if (next(p).type != TOKEN_RPAR) {
			p->error = 1;
			p->errorstring = "Unclosed parenthesis";
			goto cleanup;
		}
	} else if (tok.type == TOKEN_NUMBER) { // Vector
		ret->type = OP_VECTOR;
		ret->vector = vector(p);
	} else if (tok.type == TOKEN_IDENT) { // identifier
		ret->type = OP_IDENT;
		ret->ident = op(p);
	} else { // error
		p->error = 1;
		p->errorstring = "Unexpected token while parsing operand";
		goto cleanup;
	}

	tok = peek(p);
	if (tok.type == TOKEN_LBRACK) { // Operand [ Expr ]
		ret->hasindex = 1;
		next(p);
		ret->expr = expr(p);
		if (next(p).type != TOKEN_RBRACK) {
			p->error = 1;
			p->errorstring = "Unclosed bracket";
			goto cleanup;
		}
	} else {
		ret->hasindex = 0;
	}

	return ret;

cleanup:
	free(ret);
	return NULL;
}

// Expr
//   Operand
//   Operand Binop Expr
//   Unop Expr
struct expr {
	char *operator;
	struct operand *op;
	struct expr *expr;
};

static struct expr *expr(struct parser *p) {
	printf("expr(<%s>...)\n", peek(p).value);
	struct expr *ret = malloc(sizeof(*ret));
	ret->operator = NULL;
	ret->op = NULL;
	ret->expr = NULL;

	struct lex_item tok = peek(p);
	if (tok.type == TOKEN_EOL) {
		p->error = 1;
		p->errorstring = "Found end of line, expected expression";
		free(ret);
		return NULL;
	} else if (tok.type == TOKEN_SYM) {
		ret->operator = op(p);
		ret->expr = expr(p);
	} else {
		ret->op = operand(p);
		if (peek(p).type == TOKEN_SYM) {
			ret->operator = op(p);
			ret->expr = expr(p);
		}
	}

	return ret;
}

// Statement
//   ident '=' Expr '\n'
//   Expr '\n'
struct statement {
	int error; // If !=0, var contains the error string
	char *var;
	struct expr *expr;
};

struct statement statement(struct parser *p) {
	printf("statement(<%s>...)\n", peek(p).value);
	struct statement ret;
	struct lex_item tok = next(p);

	ret.error = 0;

	if (tok.type == TOKEN_IDENT && peek(p).type == TOKEN_EQ) {
		next(p);
		ret.var = tok.value;
		ret.expr = expr(p);
	} else {
		backup(p);
		ret.var = NULL;
		ret.expr = expr(p);
	}

	if (next(p).type != TOKEN_EOL) {
		p->error = 1;
		p->errorstring = "Unexpected tokens before EOL";
	}

	return ret;
}

struct statement *parse(struct lex_items toks) {
	struct statement *ret = malloc(sizeof(*ret));
	struct parser p;

	p.error = 0;
	p.errorstring = NULL;

	p.pos = 0;
	p.toks = toks;

	*ret = statement(&p);

	if (p.error) {
		printf("%s\n", p.errorstring);
	}

	return ret;
}
