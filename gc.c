#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>
#include "win.h"

#include "rt.h"
#include "align.h"
#include "box.h"
#include "gc.copy.h"

// TODO: Have separate edens for common types e.g. boxed integers?
//       Having separate blocks for primitives would eliminate the overhead of box headers.

extern void gc(void * (* callback)(void *, void *), void *);

typedef char * page_t __attribute__((align_value(4096)));

typedef struct {
    char * ptr;
    char * next;
} moved_t;

static size_t BLOCK_SIZE = 2*1024*1024;

static page_t eden = 0;
static char * edenNext = 0;

static page_t tenured = 0;
static char * tenuredNext = 0;

static page_t gc_initBlock() {
    return (page_t) VirtualAlloc(NULL, BLOCK_SIZE, MEM_RESERVE | MEM_COMMIT, MEM_RW);
}

typedef struct {
    char * next;
    char * beyond;
    char * start;
    char * end;
} gc_collection_t;

static bool gc_shouldCopy(const void * context, const char * p) {
    gc_collection_t * state = (gc_collection_t *) context;
    bool inBlock = p >= state->start && p < state->end;
    printf("Should%s copy %p as it is within %p - %p\n", inBlock ? "" : " NOT", p, state->start, state->end);
    return inBlock;
}

static char * gc_next(void * context, size_t size) {
    gc_collection_t * state = (gc_collection_t *) context;
    char * next = state->next;
    if (next + size > state->beyond) {
        // TODO: What about if we can allocate this but then it will be full?
        // TODO: Perform a next-generation collection
        state->next = next = tenuredNext = tenured = gc_initBlock();
        state->beyond = tenured + BLOCK_SIZE;
    }
    state->next += size;
    return next;
}

const char * gc_tenure(const char * from) {
    gc_collection_t collection = {tenuredNext, tenured + BLOCK_SIZE, eden, eden + BLOCK_SIZE};
    if (!gc_shouldCopy(&collection, from))
        return from;
    return gc_copy(&collection, &gc_next, &gc_shouldCopy, from);
}

static void * gc_collect_reference(void * unused, void * p) {
    // TODO:
    // - does p necessarily point to a header?
    return (void *) gc_tenure((char *) p);
}

static void gc_collect() {
    // TODO
    // - walk stack to find GC roots
    // - for each GC root:
    //   - gc_copy
    // What do we do when tenured gets full?
    // How can we predict it will get full ahead of time?
    if (tenured == 0) {
        tenuredNext = tenured = gc_initBlock();
    }
    gc(&gc_collect_reference, NULL);
    edenNext = eden;
}

static void gc_init() {
    edenNext = eden = gc_initBlock();
}

__attribute__((alloc_size(1))) __attribute__((returns_nonnull))
void * __cdecl malloc(size_t size) {
    size = 8 * ((size + 7) / 8); // Round up to a whole qword
    if (eden == 0)
        gc_init(); // TODO: flag as cold?
    char * aligned = alignSize(edenNext, size);
    char * allocated = aligned;
    if (aligned + size + sizeof(box_t) - eden > BLOCK_SIZE) {
        gc_collect();
        allocated = edenNext;
    }
    box_t * header = (box_t *) allocated;
    header->nother = size / 8;
    header->nref = 0;
    edenNext = allocated + size + sizeof(box_t);
    return allocated;
}
