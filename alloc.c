#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

uint64_t g_allocated = 0;
uint64_t g_max = 0;
uint64_t g_min = -1;
uint64_t g_count = 0;
uint64_t g_copied;

typedef uint16_t* ref_t;

typedef struct {
    uint32_t nother; // Number of qwords in addition to the references
    uint32_t nref; // Number of references to managed objects that follow
    uint64_t values[0];
} tree_t;

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
static void * alloc(uint64_t size) {
  g_allocated += size;
  g_max = size > g_max ? size : g_max;
  g_min = size < g_min ? size : g_min;
  ++g_count;
  return malloc(size);
}

ref_t allocInts(uint32_t nint, uint32_t nnest) {
    size_t bytes = sizeof(uint64_t) * nint + sizeof(ref_t) * nnest;
    tree_t * tree = alloc(bytes);
    tree->nother = nint;
    tree->nref = nnest;
    for (int i = 0; i < nint; ++i) {
        tree->values[i] = i + 1;
    }
    ref_t * children = (ref_t *) &tree->values[nint];
    for (int i = 0; i < nnest; ++i) {
        children[i] = allocInts(nint / 2, nnest / 3);
    }
    return (ref_t) tree;
}

void freeInts(ref_t ref) {
  // No explicit free
}

int report() {
  printf("n=%llu  min=%llu  max=%llu  total=%llu  copied=%llu\n", g_count, g_min, g_max, g_allocated, g_copied);
  return 0;
}
