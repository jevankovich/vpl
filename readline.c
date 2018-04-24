#include "readline.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_BUF 1
size_t readline(char **buf, size_t *len, size_t *readlen, size_t linelen) {
	if (*buf == NULL || *len == 0) {
		*buf = malloc(DEFAULT_BUF);
		*len = DEFAULT_BUF;
		*readlen = 0;
		linelen = 0;
	}

	if (*readlen > linelen) {
		memmove(*buf, *buf + linelen, *readlen - linelen);
	}
	*readlen = *readlen - linelen;
	linelen = 0;
	
	while (1) {
		ssize_t read_ret;
		// Search for a newline and, if found, exit the loop
		for (; linelen < *readlen; linelen++) {
			if ((*buf)[linelen] == '\n') {
				goto newline;
			}
		}

		// No new line was found, we definitely need to expand the buffer
		if (*readlen == *len) {
			*buf = realloc(*buf, (*len) * 2);
			*len *= 2;
		}

		// Read up to the end of the buffer
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

