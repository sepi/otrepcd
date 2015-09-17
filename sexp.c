#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned int symbol_t;
struct pair_s; 					/* Predeclaration */
typedef struct pair_s pair_t;

#define MAX_SYMBOLS 512
#define MAX_SYMBOL_LEN 512 /* The length including the terminating \0 */

static char *symbol_table[MAX_SYMBOLS];
symbol_t symbol_idx;

typedef enum {
	NIL_TYPE,
	PAIR_TYPE,
	SYMBOL_TYPE,
	INTEGER_TYPE,
	STRING_TYPE
} object_type_t;

typedef struct {
	object_type_t type;
	union {
		pair_t *pair;
		symbol_t symbol;
		int integer;
		char *string;
	};
} object_t;

struct pair_s {
	object_t *head;
	object_t *tail;
};

const symbol_t NULL_SYMBOL = 0;
object_t nil_; // TODO initialize
object_t *nil;

object_t *make_symbol_object(symbol_t s);
void write(object_t *a);
bool nil_p(object_t *o);
bool symbol_p(object_t *o);
bool pair_p(object_t *o);
bool string_p(object_t *o);
bool integer_p(object_t *i);

object_t *head(object_t *o);
object_t *tail(object_t *o);

void nl() { printf("\n"); }
void fatal_error(const char *message);

void
init_nil() {
	nil_.type = NIL_TYPE;
	nil = &nil_;
}

void toupper_str(char *str) {
	int i;
	for (i = 0;
		 i < MAX_SYMBOL_LEN && str[i] != '\0';
		 ++i) {
		str[i] = toupper(str[i]);
	}
}

symbol_t
insert_symbol(char *str) {
	if (symbol_idx < MAX_SYMBOLS) {
		/* FIXME: unsafe if str is not null terminated */
		int len = strlen(str)+1; /* +1 for \0 */
		char *new_mem = (char *)calloc(len, sizeof(char));
		if (new_mem == NULL) {
			fprintf(stderr, "Could not allocate\n");
			return NULL_SYMBOL;
		} else {
			symbol_table[symbol_idx] = new_mem;
			strncpy(symbol_table[symbol_idx], str, len);
			toupper_str(symbol_table[symbol_idx]);
			return symbol_idx++;
		}
	} else {
		return NULL_SYMBOL;
	}
}

void
init_symbol_table() {
	for (int i = 0; i < MAX_SYMBOLS; ++i) {
		symbol_table[i] = NULL;
	}
	symbol_idx = 0;
	insert_symbol("NULL_SYMBOL");
}

symbol_t
find_symbol(char *str) {
	char buf[MAX_SYMBOL_LEN];
	strncpy(buf, str, MAX_SYMBOL_LEN);
	toupper_str(buf);

	for (int i = 0; i < symbol_idx; ++i) {
		if (strncmp(buf, symbol_table[i], MAX_SYMBOL_LEN) == 0) {
			return i;
		}
	}

	return NULL_SYMBOL;
}

symbol_t
intern_symbol(char *sym_name) {
	symbol_t s = find_symbol(sym_name);
	if (s != NULL_SYMBOL) {
		return s;
	} else {
		/* Nothing found */
		s = insert_symbol(sym_name);
		if (s == NULL_SYMBOL) {
			fatal_error("ERROR: symbol table full.");
		} else {
			return s;
		}
	}
}

char *
symbol_str(symbol_t s) {
	if (s < MAX_SYMBOLS) {
		return symbol_table[s];
	} else {
		return NULL;
	}
}

object_t *
intern(char *name) {
	return make_symbol_object(intern_symbol(name));
}

pair_t *
pair(object_t *head, object_t *tail) {
	pair_t *c = (pair_t *)malloc(sizeof(pair_t));
	if (c == NULL) {
		fatal_error("Error: Could not allocate pair cell.");
	} else {
		c->head = head;
		c->tail = tail;
		return c;
	}
}

object_t *
make_symbol_object(symbol_t s) {
	object_t *a = (object_t *)malloc(sizeof(object_t));
	if (a == NULL) {
		fatal_error("Error: Could not allocate object.");
	} else {
		a->type = SYMBOL_TYPE;
		a->symbol = s;
		return a;
	}
}

/* Does not unintern it. i.e it will still be present in symbol table */
void
free_symbol_object(object_t *o) {
	free(o);
}

object_t *
make_integer_object(int i) {
	object_t *a = (object_t *)malloc(sizeof(object_t));
	if (a == NULL) {
		fatal_error("Error: Could not allocate object.");
	} else {
		a->type = INTEGER_TYPE;
		a->integer = i;
		return a;
	}
}

void
free_integer_object(object_t *o) {
	free(o);
}

object_t *
make_pair_object(pair_t *c) {
	object_t *a = (object_t *)malloc(sizeof(object_t));
	if (a == NULL) {
		fatal_error("Error: Could not allocate object.");
	} else {
		a->type = PAIR_TYPE;
		a->pair = c;
		return a;
	}
}

void
free_pair_object(object_t *o) {
	free(o->pair);
	free(o);
}

object_t *
make_string_object(char *str) {
	object_t *o = (object_t *)malloc(sizeof(object_t));
	if (o == NULL) {
		fatal_error("Error: Could not allocate object.");
	} else {
		o->type = STRING_TYPE;
		int str_len = strlen(str);
		char *new_str = (char *)calloc(str_len+1, sizeof(char));
		if (str == NULL) {
			fatal_error("Error: Could not allocate object.");
		}
		strncpy(new_str, str, str_len);
		o->string = str;
		return o;
	}
}

void
free_string_object(object_t *o) {
	free(o->string);
	free(o);
}

void
free_object(object_t *o) {
	switch (o->type) {
		case NIL_TYPE:
			break;
		case PAIR_TYPE:
			free_pair_object(o);
			break;
		case SYMBOL_TYPE:
			free_symbol_object(o);
			break;
		case INTEGER_TYPE:
			free_integer_object(o);
			break;
		case STRING_TYPE:
			free_string_object(o);
			break;
		default:
			printf("Error: Unknown type object 0x%p.", o);
	}
}

void
free_object_rec(object_t *o) {
	if (pair_p(o)) {
		free_object(head(o));
		free_object(tail(o));
	} else {
		free_object(o);
	}
}

object_t *
cons(object_t *head, object_t *tail) {
	return make_pair_object(pair(head, tail));
}

void
write_pair(pair_t *c) {
	printf("(");
	write(c->head);
	printf(" . ");
	write(c->tail);
	printf(")");
}

bool
nil_p(object_t *o) {
	return (o->type == NIL_TYPE);
}

bool
pair_p(object_t *o) {
	return (o->type == PAIR_TYPE);
}

bool
symbol_p(object_t *o) {
	return (o->type == SYMBOL_TYPE);
}

bool
integer_p(object_t *o) {
	return (o->type == INTEGER_TYPE);
}

bool
string_p(object_t *o) {
	return (o->type == STRING_TYPE);
}

object_t *
head(object_t *o) {
	assert(pair_p(o));
	return o->pair->head;
}

object_t *
tail(object_t *o) {
	assert(pair_p(o));
	return o->pair->tail;
}

void
fatal_error(const char *message) {
	fprintf(stderr, message);
	nl();
	exit(-1);
}

bool
proper_list_p(object_t *l) {
	if (!pair_p(l)) {
		return false;
	} else if (nil_p(tail(l))) {
		return true;
	} else {
		return proper_list_p(tail(l));
	}
}

bool
dotted_pair_p(object_t *p) {
	if (!pair_p(p)) {
		return false;
	} else if (nil_p(tail(p))) {
		return false;
	} else {
		return true;
	}
}

bool
dotted_list_p(object_t *l) {
	if (! pair_p(l)) {
		return false;
	} else { 
		if (nil_p(tail(l))) { 	/* It's a proper list */
			return false;
		} else if (! pair_p(tail(l))) { /* dotted pair */
			return true;
		} else {
			return dotted_list_p(tail(l));
		}
	}
}

void
write(object_t *a) {
	switch (a->type) {
		case NIL_TYPE:
			printf("NIL");
			break;
		case PAIR_TYPE:
			write_pair(a->pair);
			break;
		case SYMBOL_TYPE:
			printf("%s", symbol_str(a->symbol));
			break;
		case INTEGER_TYPE:
			printf("%d", a->integer);
			break;
		case STRING_TYPE:
			printf("%s", a->string);
			break;
		default:
			printf("Error: Unknown type object 0x%p.", a);
	}
}

void
write_proper_list(object_t *l) {
	printf("(");
	do {
		write(head(l));
		l = tail(l);
		if (!nil_p(l)) {
			printf(" ");
		} else {
			break;
		}
	} while (true);
	printf(")");
}

char sexp[] = "(foo (bar 1 2 3) \"faz\")";

bool
symbol_char_p(char c) {
	return (c != '(' &&
			c != ')' &&
			c != ' ' &&
			c != '\n' &&
			c != '\r' &&
			c != '\f' &&
			c != '\b' &&
			c != '\a' &&
			c != '\"');
}

object_t *
read_symbol(const char *str, int *count) {
	char buffer[MAX_SYMBOL_LEN] = {0};
	char *bufptr = buffer;
	
	while (*str != '\0') {
		if (symbol_char_p(*str)) {
			*bufptr = *str;
			(*count)++;
			str++;
			bufptr++;
		} else {
			break;
		}
	}
	return intern(buffer);
}

/* FIXME: bullshit since '-' is only allowed at beginning */
bool
integer_char_p(char c) {
	return (isdigit(c) || c == '-');
}

/* FIXME: not actually robust */
object_t *
read_integer(const char *str, int *count) {
	char buffer[MAX_SYMBOL_LEN] = {0};
	char *bufptr = buffer;

	while (*str != '\0') {
		if (integer_char_p(*str)) {
			*bufptr = *str;
			(*count)++;
			str++;
			bufptr++;
		} else {
			break;
		}
	}

	return make_integer_object(atoi(buffer));
}

object_t *
read_atom(const char *str, int *count) {
	if (symbol_char_p(*str)) {
		return read_symbol(str, count);
	} else if (integer_char_p(*str)) {
		return read_integer(str, count);
	} else {
		fatal_error("Error: Not implemented");
	}
}

object_t *
read(const char *str, int *count) {
	object_t *current_pair, *car, *cdr;
	do {
		switch (*str) {
			case '(':
				++str;
				(*count)++;
				car = read(str, count);
			case ')':
				++str;
		}
	} while (str != '\0');
}

int
main() {
	object_t *foo, *bar, *baz, *ten, *c;
	
	init_symbol_table();
	init_nil();

	foo = intern("foo");
	bar = intern("bar");
	baz = intern("baz");
	ten = make_integer_object(10);
	c = make_string_object("tets");

	c = cons(ten, nil);
	c = cons(baz, c);
	c = cons(bar, c);
	c = cons(foo, c);

	/* write(nil); */
	/* printf(" %d %d", proper_list_p(nil), dotted_list_p(nil)); nl(); */

	/* write(c); */
	/* printf(" %d %d", proper_list_p(c), dotted_list_p(c)); nl(); */
	/* write_proper_list(c); nl(); */

	/* c = cons(foo, bar); */
	/* write(c); */
	/* printf(" %d %d", proper_list_p(c), dotted_list_p(c)); nl(); */
	
	/* c = cons(foo, nil); */
	/* write(c); */
	/* printf(" %d %d", proper_list_p(c), dotted_list_p(c)); nl(); */

	int read = 0;
	c = read_atom("foo    bar", &read);
	write(c);
	printf(" %d\n", read);
	free_object_rec(c);

	c = read_atom("2100 ", &read);
	write(c);
	printf(" %d\n", read);
	free_object_rec(c);
	
	return 0;
}
