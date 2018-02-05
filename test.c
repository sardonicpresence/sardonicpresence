#include <stdint.h>
#include "prng.h"

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
extern void * alloc(uint64_t size);

extern int report();

void test(const int m, const int n) {
  void * alloca[m];
  for (int i = 0; i < n; ++i) {
    int r = random();
    int size = r % 4097;
    if (size == 0) size = r % 4*1024*1024;
    alloca[random() % m] = alloc(size);
  }
  if (random() % 1) {
    test(random() % 100, random() % 1000);
  }
}

int start() {
  // test(1, 1);
  return 0; //report();
}