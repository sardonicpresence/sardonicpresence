#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "prng.h"

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
extern void * alloc(uint64_t size);

extern int report();

static void * randomAlloc() {
    uint32_t r = random();
    uint32_t size = r % 4097;
    if (size == 0)
        size = r % (2*1024*1024);
    return alloc(size);
}

void test(const uint8_t m, const int n, const int nest) {
  void * alloca[m];
  for (int i = 0; i < m; ++i) {
      alloca[i] = NULL; //randomAlloc();
  }
  for (int i = 0; i < n; ++i) {
     int j = random() % m;
     free(alloca[j]);
     alloca[j] = randomAlloc();
  }
  if (random() & 1) {
    test(random() % 100 + 1, random() % 1000, nest+1);
  }
  if (random() & 1) {
    test(random() % 100 + 1, random() % 1000, nest+1);
  }
}

int start() {
  test(100, 1000, 0);
  return report();
}
