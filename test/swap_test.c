#include <stdint.h>
#include <assert.h>
uint64_t swap(uint64_t* a, uint64_t* b);
int main() {
	uint64_t const v1 = 2364326;
	uint64_t const v2 = 3562346;

	uint64_t t1 = v1;
	uint64_t t2 = v2;

	swap(&t1, &t2);

	assert(t1 == v2);
	assert(t2 == v1);
}
