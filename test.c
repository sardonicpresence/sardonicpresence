#include <stdint.h>
#include <stdio.h>
#include "prng.h"

__attribute__((dllimport))
void __stdcall ExitProcess(int);

typedef uint16_t* ref_t;

typedef struct {
    uint32_t nother; // Number of qwords in addition to the references
    uint32_t nref; // Number of references to managed objects that follow
    uint64_t values[0];
} tree_t;

extern ref_t allocInts(uint32_t nint, uint32_t nnest);
extern void freeInts(ref_t);
extern int report();

static void indent(int level) {
    for (int i = 0; i < level; ++i)
        printf(" ");
}

static void verifyTreeI(ref_t ref, int level) {
    // indent(level); printf("Verifying %p:\n", ref);
    if ((size_t) ref == 0xCCCCCCCCCCCCCCCC) {
        indent(level); printf(" Tree is garbage!\n");
        return;
    }

    tree_t * tree = (tree_t *) ref;
    if (tree->nother == 0xCCCCCCCC) {
        indent(level); printf(" Tree %p in garbage block!\n", ref);
        return;
    }
    // indent(level); printf(" nint=%d\n", tree->nother);

    ref_t * children = (ref_t *) &tree->values[tree->nother];
    // indent(level); printf(" children @ %p\n", children);
    for (int i = 0; i < tree->nref; ++i) {
        if (children[i] == NULL)
            continue;
        verifyTreeI(children[i], level+1);
    }
}

void verifyTree(ref_t ref) {
    verifyTreeI(ref, 0);
}

__attribute__((noinline))
void printChild(ref_t * children, int i, int nnest, ref_t child) {
    // printf("Child #%d / %d @ %p: %p\n", i, nnest, children, child);
}

void printNode(ref_t node) {
    // printf("Node @ %p\n", node);
}

extern ref_t allocTree(uint32_t nint, uint32_t nnest);

// static ref_t allocTree(uint32_t nint, uint32_t nnest) {
//     ref_t * tree = (ref_t *) allocInts(nint, nnest);
//     ref_t * children = tree + 1 + nint;
//     // ref_t * children = (ref_t *) &tree->values[nint];
//     for (int i = 0; i < nnest; ++i) {
//         ref_t child = allocTree(nint / 2, nnest / 3);
//         printChild(children, i, nnest, child);
//         children[i] = child;
//     }
//     return (ref_t) tree;
// }

static ref_t randomAlloc() {
    uint32_t r = random();
    uint32_t size = r % 4097;
    uint32_t children = random() % 100;
    return allocTree(size, children);
}

 __attribute__((noinline))
 static uint64_t sum(ref_t ints) {
    verifyTree(ints);
    tree_t * tree = (tree_t *) ints;
    // printf("nint=%d  nnest=%d\n", tree->nother, tree->nref);
    uint64_t total = 0;
    for (int i = 0; i < tree->nother; ++i) {
        total += tree->values[i];
    }
    ref_t * children = (ref_t *) &tree->values[tree->nother];
    // __builtin_debugtrap();
    for (int i = 0; i < tree->nref; ++i) {
        if (children[i] == NULL) {
            printf("null child #%d / %d\n", i, tree->nref);
            continue;
        }
        // printf("Child #%d / %d = %p\n", i, tree->nref, children[i]);
        total += sum(children[i]);
    }
    return total;
}

uint64_t test() {
    if (random() % 100 == 0)
        return 0;
    // printf("Allocating tree #1...\n"); fflush(stdout);
    // ref_t t1 = allocTree(1000, 20);
    // printf("Summing...\n"); fflush(stdout);
    // return sum(t1);

    printf("Allocating tree #1...\n"); fflush(stdout);
    ref_t t1 = randomAlloc();
    printf("Allocating tree #2...\n"); fflush(stdout);
    ref_t t2 = randomAlloc();
    // ref_t t3 = allocInts(2*1024*1024 / 8 - 1, 0);
    uint64_t sum1 = sum(t1);
    freeInts(t1);
    printf("Recursing...\n"); fflush(stdout);
    uint64_t nested = test();
    // printf("Summing...\n"); fflush(stdout);
    uint64_t total = nested + sum1 + sum(t2); // + sum(t3);
    freeInts(t2);
    return total;
}

int start() {
  uint64_t total = test();
  printf("total = %llu\n", total);
  int exitCode = report();
  ExitProcess(exitCode);
  return exitCode;
}
