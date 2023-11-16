#include <assert.h>
#include <stdint.h>
uint64_t increment(uint64_t value);
int main() {
	for (int i = 0; i < 100; ++i) {
		assert(increment(i) == i + 1);
	}
}
