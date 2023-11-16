#include <stdint.h>
#include <stdio.h>
uint64_t fib(uint64_t n);
int main() {
	for (int i = 1; i < 10; ++i) {
		printf("fib(%d) = %lu\n", i, fib(i));
	}
}
