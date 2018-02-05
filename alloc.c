#include <stdint.h>

uint64_t g_allocated = 0;
uint64_t g_max = 0;
uint64_t g_min = 0x7FFFFFFFFFFFFFFF;
uint64_t g_count = 0;

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
void * alloc(uint64_t size) {
  g_allocated += size;
  g_max = size > g_max ? size : g_max;
  g_min = size < g_min ? size : g_min;
  ++g_count;
  return (void *) 0x1; // TODO
}

int report() {
  return 0;
}
