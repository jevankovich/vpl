#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "readline.h"
#include "scan.h"

void print_tok(struct scan_item item) {
	if (item.type == SCAN_EOL) {
		write(1, "<\\n>", 4);
	} else if (item.type == SCAN_SPACE) {
		write(1, " ", 1);
	} else {
		write(1, "<", 1);
		write(1, item.value, strlen(item.value));
		write(1, ">", 1);
	}
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
