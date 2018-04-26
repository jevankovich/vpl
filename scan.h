#ifndef SCAN_H
#define SCAN_H
// scan implements a lexer that follows the structure shown in:
//     https://www.youtube.com/watch?v=HxaD_trXwRE
// Instead of a Go channel, a callback is used to use or collect the tokens.

#include <stddef.h>

enum token_type {
	TOKEN_ERROR,
	TOKEN_EOL,

	TOKEN_IDENT,
	TOKEN_NUMBER,

	TOKEN_LPAR,
	TOKEN_RPAR,

	TOKEN_LBRACK,
	TOKEN_RBRACK,

	TOKEN_EQ,
	TOKEN_SYM
};

struct lex_item {
	enum token_type type;
	char *value;
};

struct lex_items {
	size_t cap;
	size_t len;
	struct lex_item *items;
};

void free_items(struct lex_items *items);
void *collect(struct lex_item, void *);

void *scan(char *, void *(*)(struct lex_item, void *), void *);
#endif
