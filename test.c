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
    uint32_t children = random() % 10;
    if (size == 0) {
        size = r % (2*1024*1024 - 8);
        children = 0;
    }
    return allocInts(size, children);
}

 __attribute__((noinline))
 static uint64_t sum(ref_t ints) {
    uint32_t * tree = (uint32_t *) ints;
    printf("nother=%d  nref=%d\n", tree[0], tree[1]);
    __builtin_debugtrap();
    uint64_t * values = (uint64_t *) (tree + 2);
    uint64_t total = 0;
    for (int i = 0; i < tree[0]; ++i) {
        total += values[i];
    }
    ref_t * children = (ref_t *) &values[tree[0]];
    for (int i = 0; i < tree[1]; ++i) {
        total += sum(children[i]);
    }
    return total;
}

uint64_t test() {
    if (random() % 10 == 0)
        return 0;
    ref_t t1 = randomAlloc();
    return sum(t1);
    // ref_t t2 = randomAlloc();
    // uint64_t nested = test();
    // return nested + sum(t1) + sum(t2);
}

int start() {
  uint64_t total = test();
  printf("total = %llu\n", total);
  return report();
}
