#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define DEFAULT_BUF 1
size_t readline(char **buf, size_t *len, size_t *readlen, size_t linelen) {
	if (*buf == NULL || *len == 0) {
		*buf = malloc(DEFAULT_BUF);
		*len = DEFAULT_BUF;
	}

	if (*readlen > linelen) {
		memmove(*buf, *buf + linelen, *readlen - linelen);
	}

	*readlen = *readlen - linelen;

	linelen = 0;
	while (1) {
		ssize_t read_ret;
		for (; linelen < *readlen; linelen++) {
			if ((*buf)[linelen] == '\n') {
				goto newline;
			}
		}

		if (*readlen == *len) {
			*buf = realloc(*buf, (*len) * 2);
			*len *= 2;
		}

		read_ret = read(0, *buf + *readlen, *len - *readlen);
		if (read_ret <= 0) {
			return 0;
		}
		*readlen += read_ret;
	}

newline:
	linelen++; // Count the \n too
	return linelen;
}

enum token {
	SPACE_TOK,
	NUMBER_TOK,
	IDENT_TOK,
	NEWLINE_TOK
};

size_t space_tok(char *line) {
	size_t i;
	for (i = 0; (line[i] == ' ' || line[i] == '\t') && line[i] != '\n'; i++) {}
	return i;
}

size_t number_tok(char *line) {
	size_t i;
	for (i = 0; line[i] != ' ' && line[i] != '\t' && line[i] != '\n'; i++) {}
	return i;
}

size_t ident_tok(char *line) {
	size_t i;
	for (i = 0; line[i] != ' ' && line[i] != '\t' && line[i] != '\n'; i++) {}
	return i;
}

void scan(char *line) {
	while (1) {
		size_t len = 0;
		if (*line == ' ' || *line == '\t') {
			len = space_tok(line);
			write(1, "_\n", 2);
		} else if (*line >= '0' && *line <= '9') {
			len = number_tok(line);
			write(1, line, len);
			write(1, "\n", 1);
		} else if (*line == '\n') {
			write(1, "\\n\n", 3);
			break;
		} else {
			len = ident_tok(line);
			write(1, line, len);
			write(1, "\n", 1);
		}

		line += len;
	}
}

int main() {
	char *buf = NULL;
	size_t buflen = 0;
	size_t readlen = 0;
	size_t linelen = 0;

	while (1) {
		write(1, "> ", 2);
		linelen = readline(&buf, &buflen, &readlen, linelen);
		if (linelen <= 0) {
			break;
		}

		scan(buf);
	}
}
