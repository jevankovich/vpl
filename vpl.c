#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "scan.h"

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

void print_tok(struct scan_item item) {
	write(1, "<", 1);
	if (item.type == SCAN_EOL) {
		write(1, "\\n", 2);
	} else {
		write(1, item.value, strlen(item.value));
	}
	write(1, ">", 1);
	free(item.value);
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

		scan(buf, print_tok);
		write(1, "\n", 1);
	}
}
