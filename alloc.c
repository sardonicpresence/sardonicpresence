#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

uint64_t g_allocated = 0;
uint64_t g_max = 0;
uint64_t g_min = 0x7FFFFFFFFFFFFFFF;
uint64_t g_count = 0;
uint64_t g_copied;

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
void * alloc(uint64_t size) {
  g_allocated += size;
  g_max = size > g_max ? size : g_max;
  g_min = size < g_min ? size : g_min;
  ++g_count;
  return malloc(size);
}

int report() {
  printf("n=%llu  min=%llu  max=%llu  total=%llu  copied=%llu\n", g_count, g_min, g_max, g_allocated, g_copied);
  return 0;
}
