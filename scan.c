#include "scan.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct state;
typedef struct state (state_func)(struct state, scan_func);
struct state {
	state_func *fn;
	char *buf;
};

state_func start, eol, space, ident, numeric, sym;

void emitn(scan_func fn, enum scan_type type, char *str, size_t n) {
	char *buf = malloc(n + 1);
	memcpy(buf, str, n);
	buf[n] = '\0';

	struct scan_item item = {type, buf};
	fn(item);
}
	
void error(scan_func fn, char *errstring) {
	emitn(fn, SCAN_ERROR, errstring, strlen(errstring));
}

int issym(char c) {
	static const char *syms = "=|&*^-+/";
	for (int i = 0; syms[i]; i++) {
		if (c == syms[i]) {
			return 1;
		}
	}
	return 0;
}

struct state start(struct state st, scan_func fn) {
	if (*(st.buf) == '\n') {
		st.fn = eol;
	} else if (isspace(*(st.buf))) {
		st.fn = space;
	} else if (isalpha(*(st.buf))) {
		st.fn = ident;
	} else if (isdigit(*(st.buf))) {
		st.fn = numeric;
	} else if (issym(*(st.buf))) {
		st.fn = sym;
	} else {
		error(fn, "Unexpected character in input");
		st.fn = NULL;
	}
		
	return st;
}

struct state eol(struct state st, scan_func fn) {
	emitn(fn, SCAN_EOL, st.buf, 1);
	
	st.fn = NULL;
	st.buf++;
	return st;
}

struct state space(struct state st, scan_func fn) {
	size_t i;
	for (i = 0; st.buf[i] != '\n' && isspace(st.buf[i]); i++) {}
	emitn(fn, SCAN_SPACE, st.buf, i);
	
	st.fn = start;
	st.buf += i;
	return st;
}

struct state ident(struct state st, scan_func fn) {
	size_t i;
	for (i = 0; !issym(st.buf[i]) && !isspace(st.buf[i]); i++) {}
	emitn(fn, SCAN_SPACE, st.buf, i);

	st.fn = start;
	st.buf += i;
	return st;
}

struct state sym(struct state st, scan_func fn) {
	enum scan_type type;

	switch (*(st.buf)) {
	case '=':
		type = SCAN_EQ;
		break;
	case '|':
		type = SCAN_PIPE;
		break;
	case '&':
		type = SCAN_AND;
		break;
	case '*':
		type = SCAN_STAR;
		break;
	case '^':
		type = SCAN_CARET;
		break;
	case '-':
		type = SCAN_MINUS;
		break;
	case '+':
		type = SCAN_PLUS;
		break;
	case '/':
		type = SCAN_SLASH;
		break;
	default:
		error(fn, "How did this even happen?");
		st.fn = NULL;
		st.buf++;
		return st;
	}
	emitn(fn, type, st.buf, 1);
	st.fn = start;
	st.buf++;

	return st;
}

struct state numeric(struct state st, scan_func fn) {
	size_t i;
	for (i = 0; !issym(st.buf[i]) && !isspace(st.buf[i]); i++) {}
	emitn(fn, SCAN_SPACE, st.buf, i);

	st.fn = start;
	st.buf += i;
	return st;
}

void scan(char *line, scan_func handle) {
	struct state st = {&start, line};
	while (st.fn) {
		st = st.fn(st, handle);
	}
}
