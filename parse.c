#include "parse.h"
#include "scan.h"

struct parser {
	int error;
	char *errorstring;

	size_t pos;
	struct lex_items toks;
};

struct lex_item peek(struct parser *p) {
	return p->lex_items[p->pos];
}

struct lex_item next(struct parser *p) {
	return p->lex_items[p->pos++];
}

void backup(struct parser *p) {
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

char *op(struct parser *p) {
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

struct vector *vector(struct parser *p) {
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
		struct expr *expr // ( Expr )
		struct vector *vector; // Vector
		char *ident; // identifier
	};

	int hasindex;
	struct expr *index; // Operand [ Expr ]
};

struct operand *operand(struct parser *p) {
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

struct expr *expr(struct parser *p) {
	struct expr *ret = malloc(sizeof(*ret));
	ret->operator = NULL;
	ret->op = NULL;
	ret->expr = NULL;

	struct lex_item tok = peek(p);
	if (tok.type == TOKEN_SYM) {
		ret->operator = op(p);
		ret->expr = expr(p);
	} else {
		ret->operand = operand(p);
		if (peek(p).type == TOKEN_SYM) {
			ret->operator = op(p);
			ret->expr = expr(p);
		}
	}

	return ret;
}

// Statement
//   ident '=' Expr
//   Expr
struct statement statement(struct parser *p) {
	next(p);
	if (peek(p).type == TOKEN_EQ) {
	}
}
