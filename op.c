#include "op.h"
#include "parse.h"

#include <stdlib.h>

struct value nop(struct value x) {
	return x;
}

struct value inv(struct value x) {
	size_t len = 1;
	for (size_t i = 0; i < x.dims; i++) {
		len *= x.size[i];
	}

	for (size_t i = 0; i < len; i++) {
		x.data[i] = 1 / x.data[i];
	}

	return x;
}

struct value neg(struct value x) {
	size_t len = 1;
	for (size_t i = 0; i < x.dims; i++) {
		len *= x.size[i];
	}

	for (size_t i = 0; i < len; i++) {
		x.data[i] = -x.data[i];
	}

	return x;
}

struct value mul(struct value x, struct value y) {
	size_t xlen = 1;
	size_t ylen = 1;
	for (size_t i = 0; i < x.dims; i++) {
		xlen *= x.size[i];
	}
	
	for (size_t i = 0; i < y.dims; i++) {
		ylen *= y.size[i];
	}

	if (xlen != ylen) {
		free(x.size);
		free(x.data);
		free(y.size);
		free(y.data);

		x.dims = 0;
		x.size = NULL;
		x.data = NULL;
		return x;
	}

	for (size_t i = 0; i < xlen; i++) {
		x.data[i] *= y.data[i];
	}

	free(y.size);
	free(y.data);
	return x;
}

struct value divide(struct value x, struct value y) {
	size_t xlen = 1;
	size_t ylen = 1;
	for (size_t i = 0; i < x.dims; i++) {
		xlen *= x.size[i];
	}
	
	for (size_t i = 0; i < y.dims; i++) {
		ylen *= y.size[i];
	}

	if (xlen != ylen) {
		free(x.size);
		free(x.data);
		free(y.size);
		free(y.data);

		x.dims = 0;
		x.size = NULL;
		x.data = NULL;
		return x;
	}

	for (size_t i = 0; i < xlen; i++) {
		x.data[i] /= y.data[i];
	}

	free(y.size);
	free(y.data);
	return x;
}

struct value add(struct value x, struct value y) {
	size_t xlen = 1;
	size_t ylen = 1;
	for (size_t i = 0; i < x.dims; i++) {
		xlen *= x.size[i];
	}
	
	for (size_t i = 0; i < y.dims; i++) {
		ylen *= y.size[i];
	}

	if (xlen != ylen) {
		free(x.size);
		free(x.data);
		free(y.size);
		free(y.data);

		x.dims = 0;
		x.size = NULL;
		x.data = NULL;
		return x;
	}

	for (size_t i = 0; i < xlen; i++) {
		x.data[i] += y.data[i];
	}

	free(y.size);
	free(y.data);
	return x;
}

struct value sub(struct value x, struct value y) {
	size_t xlen = 1;
	size_t ylen = 1;
	for (size_t i = 0; i < x.dims; i++) {
		xlen *= x.size[i];
	}
	
	for (size_t i = 0; i < y.dims; i++) {
		ylen *= y.size[i];
	}

	if (xlen != ylen) {
		free(x.size);
		free(x.data);
		free(y.size);
		free(y.data);

		x.dims = 0;
		x.size = NULL;
		x.data = NULL;
		return x;
	}

	for (size_t i = 0; i < xlen; i++) {
		x.data[i] -= y.data[i];
	}

	free(y.size);
	free(y.data);
	return x;
}

struct value at(struct value x, struct value y) {
	size_t xlen = 1, ylen = 1;
	for (size_t i = 0; i < x.dims; i++) {
		xlen *= x.size[i];
	}
	
	for (size_t i = 0; i < y.dims; i++) {
		ylen *= y.size[i];
	}

	for (size_t i = 0; i < ylen; i++) {
		size_t j = y.data[i];
		if (j < xlen) {
			y.data[i] = x.data[j];
		} else {
			free(x.size);
			free(x.data);
			free(y.size);
			free(y.data);

			return (struct value){0, NULL, NULL};
		}
	}

	free(x.size);
	free(x.data);

	return y;
}
