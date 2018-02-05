#include <stdint.h>

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
void * alloc(uint64_t size) {
  return (void *) 0x1; // TODO
}

int report() {
  return 0;
}
