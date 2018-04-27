#include "parse.h"
#include "scan.h"

#include <stdlib.h>

struct parser;

struct vector;
struct operand;
struct expr;
struct statement;

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
	struct expr *ret = malloc(sizeof(*ret));
	ret->operator = NULL;
	ret->op = NULL;
	ret->expr = NULL;

	struct lex_item tok = peek(p);
	if (tok.type == TOKEN_SYM) {
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
struct statement statement(struct parser *p) {
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

	return ret;
}
