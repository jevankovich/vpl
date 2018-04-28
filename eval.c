#include "parse.h"
#include "scan.h"
#include "op.h"

#include <stdlib.h>
#include <stdio.h>

struct parser;

static unop (*parse_unop(struct parser *));
static binop (*parse_binop(struct parser *));
static struct value vector(struct parser *);
static struct value operand(struct parser *);
static struct value expr(struct parser *);
static struct value statement(struct parser *);

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

static void error(struct parser *p, char *errorstring) {
	p->error = 1;
	p->errorstring = errorstring;
}

static unop (*parse_unop(struct parser *p)) {
	printf("parse_unop(<%s>...)\n", peek(p).value);
	struct lex_item tok = next(p);
	if (tok.type == TOKEN_SYM) {
		switch (tok.value[0]) {
			case '*':
				return nop;
			case '/':
				return inv;
			case '+':
				return nop;
			case '-':
				return neg;
			default:
				error(p, "Unrecognized symbol");
				return NULL;
		}
	} else if (tok.type == TOKEN_IDENT) {
		error(p, "Variables not yet implemented");
		return NULL;
	} else {
		error(p, "Operation must be symbol or identifier");
		return NULL;
	}
}

static binop (*parse_binop(struct parser *p)) {
	printf("parse_binop(<%s>...)\n", peek(p).value);
	struct lex_item tok = next(p);
	if (tok.type == TOKEN_SYM) {
		switch (tok.value[0]) {
			case '*':
				return mul;
			case '/':
				return divide;
			case '+':
				return add;
			case '-':
				return sub;
			default:
				error(p, "Unrecognized symbol");
				return NULL;
		}
	} else if (tok.type == TOKEN_IDENT) {
		error(p, "Variables not yet implemented");
		return NULL;
	} else {
		error(p, "Operation must be symbol or identifier");
		return NULL;
	}
}

struct number_list {
	double value;
	struct number_list *next;
};

static struct number_list *parse_array(struct parser *p) {
	if (peek(p).type != TOKEN_NUMBER) {
		error(p, "Expected a number");
		return NULL;
	}

	struct number_list *ret = malloc(sizeof(*ret));
	ret->value = atof(next(p).value);
	
	if (peek(p).type == TOKEN_NUMBER) {
		ret->next = parse_array(p);
	} else {
		ret->next = NULL;
	}

	return ret;
}

static struct value vector(struct parser *p) {
	printf("vector(<%s>...)\n", peek(p).value);
	struct number_list *l = parse_array(p);
	if (l == NULL) {
		return (struct value){0, NULL, NULL};
	}

	size_t len = 0;
	for (struct number_list *cursor = l; cursor != NULL; cursor = cursor->next) {
		len++;
	}

	struct value ret;
	ret.dims = 1;
	ret.size = malloc(sizeof(*(ret.size)));
	ret.size[0] = len;
	ret.data = malloc(len * sizeof(*(ret.data)));

	for (size_t i = 0; i < len; i++) {
		struct number_list *prev = l;
		ret.data[i] = l->value;

		l = l->next;
		free(prev);
	}

	return ret;
}

// Operand
//   ( Expr )
//   Vector
//   identifier
//   Operand [ Expr ]
static struct value operand(struct parser *p) {
	printf("operand(<%s>...)\n", peek(p).value);
	struct lex_item tok = peek(p);
	struct value ret;
	if (tok.type == TOKEN_LPAR) {
		next(p);
		ret = expr(p);
		if (next(p).type != TOKEN_RPAR) {
			error(p, "Unmatched left parenthesis");
		}
	} else if (tok.type == TOKEN_NUMBER) {
		ret = vector(p);
	} else if (tok.type == TOKEN_IDENT) {
		error(p, "Variables not yet supported");
	} else {
		error(p, "Operand must be a number, parenthesized expression, or variable");
	}

	if (peek(p).type == TOKEN_RBRACK) {
		next(p);
		struct value idx = expr(p);
		if (next(p).type != TOKEN_RBRACK) {
			error(p, "Unmatched left bracket");
		}

		ret = at(ret, idx);
	}

	return ret;
}

// Expr
//   Operand
//   Operand Binop Expr
//   Unop Expr
static struct value expr(struct parser *p) {
	printf("expr(<%s>...)\n", peek(p).value);
	struct lex_item tok = peek(p);
	if (tok.type == TOKEN_SYM) {
		unop *op = parse_unop(p);
		return op(expr(p));
	}

	// TODO: Verify this logic
	if (tok.type == TOKEN_IDENT) {
		next(p);
		if (peek(p).type != TOKEN_EOL) {
			backup(p);
			unop *op = parse_unop(p);
			return op(expr(p));
		}

		backup(p);
		return operand(p);
	}

	struct value x = operand(p);

	tok = peek(p);
	if (tok.type == TOKEN_IDENT || tok.type == TOKEN_SYM) {
		binop *op = parse_binop(p);
		return op(x, expr(p));
	}

	error(p, "Malformed expression");
	return (struct value){0, NULL, NULL};
}

// Statement
//   ident '=' Expr '\n'
//   Expr '\n'
static struct value statement(struct parser *p) {
	printf("statement(<%s>...)\n", peek(p).value);
	struct value ret;
	struct lex_item tok = next(p);

	if (tok.type == TOKEN_IDENT && peek(p).type == TOKEN_EQ) {
		next(p);
		ret = expr(p);
	} else {
		backup(p);
		ret = expr(p);
	}

	if (next(p).type != TOKEN_EOL) {
		error(p, "Unexpected tokens before EOL");
	}

	return ret;
}

struct value parse(struct lex_items toks) {
	struct value ret;
	struct parser p;

	p.error = 0;
	p.errorstring = NULL;

	p.pos = 0;
	p.toks = toks;

	ret = statement(&p);

	if (p.error) {
		printf("%s\n", p.errorstring);
	}

	return ret;
}
