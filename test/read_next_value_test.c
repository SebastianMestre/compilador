#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

struct list {
	uint64_t value;
	struct list* next;
};

struct list* cons(uint64_t value, struct list* next) {
	struct list* result = malloc(sizeof(*result));
	*result = (struct list) {.value = value, .next = next};
	return result;
}

uint64_t read_next_value(struct list* list);

int main() {
	struct list* l = cons(15, cons(77, NULL));
	printf("second value: %ld\n", read_next_value(l));
}
