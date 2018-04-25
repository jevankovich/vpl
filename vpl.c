#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "readline.h"
#include "scan.h"

void print_tok(struct lex_item item) {
	if (item.type == TOKEN_EOL) {
		write(1, "<\\n>", 4);
	} else {
		write(1, "<", 1);
		write(1, item.value, strlen(item.value));
		write(1, ">", 1);
	}
}

int main() {
	char *buf = NULL;
	size_t buflen = 0;
	size_t readlen = 0;
	size_t linelen = 0;

	struct lex_items items;
	items.cap = 0;
	items.len = 0;
	items.items = NULL;

	while (1) {
		write(1, "> ", 2);
		linelen = readline(&buf, &buflen, &readlen, linelen);
		if (linelen <= 0) {
			break;
		}

		items.len = 0;
		scan(buf, collect, &items);

		for (size_t i = 0; i < items.len; i++) {
			print_tok(items.items[i]);
		}
		write(1, "\n", 1);
	}
}
