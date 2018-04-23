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
		for (; linelen < *readlen; linelen++) {
			if ((*buf)[linelen] == '\n') {
				goto newline;
			}
		}

		if (*readlen == *len) {
			*buf = realloc(*buf, (*len) * 2);
			*len *= 2;
		}

		ssize_t read_ret = read(0, *buf + *readlen, *len - *readlen);
		if (read_ret <= 0) {
			return 0;
		}
		*readlen += read_ret;
		//*readlen += read(0, *buf + *readlen, *len - *readlen);
	}

newline:
	linelen++; // Count the \n too
	return linelen;
}

const char *test = "hi there\na\n";
int main() {
	char *buf = NULL;
	size_t buflen = 0;
	size_t readlen = 0;
	size_t linelen = 0;

	/*
	buf = malloc(512);
	strcpy(buf, test);
	buflen = 512;
	readlen = strlen(test);
	linelen = 0;
	*/

	while (1) {
		write(1, "> ", 2);
		linelen = readline(&buf, &buflen, &readlen, linelen);
		write(1, buf, linelen);
		if (linelen <= 0) {
			break;
		}
	}
}
