#include <stdint.h>
#include <stdio.h>
#include "prng.h"

typedef uint16_t* ref_t;

typedef struct {
    uint32_t nother; // Number of qwords in addition to the references
    uint32_t nref; // Number of references to managed objects that follow
    uint64_t values[0];
} tree_t;

extern ref_t allocInts(uint32_t nint, uint32_t nnest);
extern void freeInts(ref_t);
extern int report();

static ref_t randomAlloc() {
    uint32_t r = random();
    uint32_t size = r % 4097;
    uint32_t children = random() % 100;
    return allocInts(size, children);
}

 __attribute__((noinline))
 static uint64_t sum(ref_t ints) {
    tree_t * tree = (tree_t *) ints;
    // printf("nint=%d  nnest=%d\n", tree->nother, tree->nref);
    uint64_t total = 0;
    for (int i = 0; i < tree->nother; ++i) {
        total += tree->values[i];
    }
    ref_t * children = (ref_t *) &tree->values[tree->nother];
    for (int i = 0; i < tree->nref; ++i) {
        total += sum(children[i]);
    }
    return total;
}

uint64_t test() {
    if (random() % 100 == 0)
        return 0;
    ref_t t1 = randomAlloc();
    ref_t t2 = randomAlloc();
    ref_t t3 = allocInts(2*1024*1024 / 8 - 1, 0);
    uint64_t nested = test();
    return nested + sum(t1) + sum(t2) + sum(t3);
}

int start() {
  uint64_t total = test();
  printf("total = %llu\n", total);
  return report();
}
