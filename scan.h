#ifndef SCAN_H
#define SCAN_H
enum scan_type {
	SCAN_ERROR,
	SCAN_EOL,
	SCAN_SPACE,

	SCAN_IDENT,
	SCAN_NUMBER,

	SCAN_LPAR,
	SCAN_RPAR,

	SCAN_EQ,
	SCAN_PIPE,
	SCAN_AND,
	SCAN_STAR,
	SCAN_CARET,
	SCAN_MINUS,
	SCAN_PLUS,
	SCAN_SLASH
};

struct scan_item {
	enum scan_type type;
	char *value;
};

typedef void (*scan_func)(struct scan_item);

void scan(char *, scan_func);
#endif
