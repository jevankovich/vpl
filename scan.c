#include "scan.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct lex_state;
typedef void (state_func)(struct lex_state *);
struct lex_state {
	state_func *fn;

	char *buf;
	size_t start;
	size_t pos;
	
	void *(*callback)(struct lex_item, void *);
	void *userdata;
};

static void emit(struct lex_state *st, enum token_type t) {
	size_t len;
	struct lex_item tok;
	
	tok.type = t;

	len = st->pos - st->start;
	tok.value = malloc(len + 1);

	memcpy(tok.value, st->buf + st->start, len);
	tok.value[len] = '\0';

	st->start = st->pos;

	st->userdata = st->callback(tok, st->userdata);
}

static void ignore(struct lex_state *st) {
	st->start = st->pos;
}

static void error(struct lex_state *st, char *errstring) {
	size_t len;
	struct lex_item tok;

	tok.type = TOKEN_ERROR;

	len = strlen(errstring);
	tok.value = malloc(len + 1);

	memcpy(tok.value, errstring, len);
	tok.value[len] = '\0';

	st->userdata = st->callback(tok, st->userdata);

	st->fn = NULL;
}

static char next(struct lex_state *st) {
	return st->buf[st->pos++];
}

static char peek(struct lex_state *st) {
	return st->buf[st->pos];
}

static void backup(struct lex_state *st) {
	st->pos--;
}

static int accept(struct lex_state *l, const char *valid) {
	char c = next(l);
	for (size_t i = 0; valid[i]; i++) {
		if (c == valid[i]) return 1;
	}

	backup(l);
	return 0;
}

static void acceptrun(struct lex_state *l, const char *valid) {
	while (accept(l, valid)) {}
}

static int acceptf(struct lex_state *l, int (*pred)(int)) {
	if (pred(next(l))) return 1;
	backup(l);

	return 0;
}

static void acceptrunf(struct lex_state *l, int (*pred)(int)) {
	while (acceptf(l, pred)) {}
}

static state_func start;
static state_func eol;
static state_func space;
static state_func ident;
static state_func numeric;
static state_func sym;

static void start(struct lex_state *l) {
	char c = peek(l);
	if (c == '\n' || c == '\r') {
		l->fn = eol;
	} else if (isspace(c)) {
		l->fn = space;
	} else if (isdigit(c) || c == '+' || c == '-') {
		l->fn = numeric;
	} else if (isalpha(c)) {
		l->fn = ident;
	} else {
		l->fn = sym;
	}
}

static void eol(struct lex_state *l) {
	acceptrun(l, "\r\n");
	emit(l, TOKEN_EOL);

	l->fn = NULL;
}

static void space(struct lex_state *l) {
	acceptrun(l, "\t\f\v ");
	ignore(l);

	l->fn = start;
}

static void ident(struct lex_state *l) {
	acceptf(l, isalpha);
	acceptrunf(l, isalnum);
	emit(l, TOKEN_IDENT);

	l->fn = start;
}

static void numeric(struct lex_state *l) {
	if (accept(l, "+-") && !acceptf(l, isdigit)) {
		l->fn = sym;
		backup(l);
		return;
	}

	acceptrunf(l, isdigit);
	if (accept(l, ".")) {
		acceptrunf(l, isdigit);
	}

	if (accept(l, "eE")) {
		accept(l, "+-");
		if (!acceptf(l, isdigit)) {
			error(l, "Scientific notation must be followed by a number");
			return;
		}
		acceptrunf(l, isdigit);
	}

	if (isalnum(peek(l))) {
		error(l, "A number must be separated from an identifier by a space");
		return;
	}

	emit(l, TOKEN_NUMBER);

	if (peek(l) == '+' || peek(l) == '-') {
		l->fn = sym;
	} else {
		l->fn = start;
	}
}

static void sym(struct lex_state *l) {
	l->fn = start;

	if (accept(l, "*/+-")) {
		emit(l, TOKEN_SYM);
	} else if (accept(l, "=")) {
		emit(l, TOKEN_EQ);
	} else if (accept(l, "(")) {
		emit(l, TOKEN_LPAR);
	} else if (accept(l, ")")) {
		emit(l, TOKEN_RPAR);
	} else if (accept(l, "[")) {
		emit(l, TOKEN_LBRACK);
	} else if (accept(l, "]")) {
		emit(l, TOKEN_RBRACK);
	} else {
		error(l, "Unknown symbol found");
	}

	return;
}

void free_items(struct lex_items *items) {
	for (int i = 0; i < items->len; i++) {
		free(items->items[i].value);
	}
	items->len = 0;
}

void *scan(char *line, void *(*callback)(struct lex_item, void *), void *userdata) {
	struct lex_state l;
	l.fn = start;
	
	l.buf = line;
	l.start = 0;
	l.pos = 0;

	l.callback = callback;
	l.userdata = userdata;

	while (l.fn != NULL) {
		l.fn(&l);
	}

	return l.userdata;
}

void *collect(struct lex_item tok, void *userdata) {
	struct lex_items *items;
	if (userdata == NULL) {
		userdata = malloc(sizeof(struct lex_items));
		items = (struct lex_items *)userdata;

		items->cap = 1;
		items->len = 0;
		items->items = malloc(sizeof(struct lex_item));
	}
	items = (struct lex_items *)userdata;

	if (items->cap == 0 || items->items == NULL) {
		items->cap = 1;
		items->len = 0;
		items->items = malloc(sizeof(struct lex_item));
	}

	if (items->len == items->cap) {
		items->items = realloc(items->items, 2 * items->cap * sizeof(struct lex_item));
		items->cap *= 2;
	}

	items->items[items->len++] = tok;

	return items;
}
