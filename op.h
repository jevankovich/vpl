#ifndef OP_H
#define OP_H
#include "parse.h"

typedef struct value unop(struct value);
typedef struct value binop(struct value, struct value);

unop nop;
unop inv;
unop neg;

binop mul;
binop divide;
binop add;
binop sub;

struct value at(struct value, struct value);

#endif
