#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint16_t* ref_t;

static size_t allocated = 0;
static size_t min = -1;
static size_t max = 0;

typedef struct {
    uint32_t nother; // Number of qwords in addition to the references
    uint32_t nref; // Number of references to managed objects that follow
    uint64_t values[0];
} tree_t;

ref_t allocInts(uint32_t nint, uint32_t nnest) {
    size_t bytes = sizeof(uint32_t) * 2 + sizeof(uint64_t) * nint + sizeof(ref_t) * nnest;
    tree_t * tree = malloc(bytes);

    allocated += bytes;
    if (bytes < min) min = bytes;
    if (bytes > max) max = bytes;

    tree->nother = nint;
    tree->nref = nnest;
    for (int i = 0; i < nint; ++i) {
        tree->values[i] = i + 1;
    }
    // ref_t * children = (ref_t *) &tree->values[nint];
    // for (int i = 0; i < nnest; ++i) {
    //     children[i] = allocInts(nint / 2, nnest / 3);
    // }
    return (ref_t) tree;
}

void freeInts(ref_t ref) {
    tree_t * tree = (tree_t *) ref;
    ref_t * children = (ref_t *) &tree->values[tree->nother];
    for (int i = 0; i < tree->nref; ++i) {
        free(children[i]);
    }
    free(tree);
}

int report() {
    printf("allocated=%llu  min=%llu,  max=%llu\n", allocated, min, max);
    return 0;
}
