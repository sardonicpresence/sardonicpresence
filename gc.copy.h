/**
 * Copy-collection functionality.
 */
#pragma once

#include "rt.h"
#include "box.h"
#include <stdio.h>

extern uint64_t g_copied;

typedef struct {
    const char * parent;
    char * (*whereTo)(void *, size_t);
    bool (*shouldCopy)(const void *, const char *);
    void * context;
    int level;
} copy_state_t;

static char * gc_copy_nonref(void * context, char * (*whereTo)(void * context, size_t size), const char * from) {
    size_t size = glc_box_sizeof(from);
    size_t toCopy = glc_box_copysize(from);
    char * to = (*whereTo)(context, size);
    memcpy(to, from, toCopy);
    g_copied += toCopy;
    return to;
}

static void * gc_copy_reference(void * context, const char * from, size_t offset) {
    copy_state_t * state = (copy_state_t *) context;
    const char * parent = state->parent;
    const box_t * box = (box_t *) parent;

    // for (int i = 0; i < state->level; ++i)
    //     printf(" ");
    int ichild = (offset - 8 * (1 + box->nother)) / 8;
    // printf("%d/%d: ", ichild, box->nref);

    if (from == NULL) {
        // printf("<null>\n");
        return context;
    }

    // TODO: Consider tagging/untagging
    const char * to = from;
    if ((*state->shouldCopy)(state->context, from)) {
        to = gc_copy_nonref(state->context, state->whereTo, from);
        // printf("%p -> %p\n", from, to);
    } else {
        // printf("%p (not relocatng)\n", to);
        // return state;
    }

    // Copy referenced objects, updating references
    *((const char **) (parent + offset)) = to;
    state->parent = to;
    ++state->level;
    glc_box_walkobj(state, &gc_copy_reference, from);
    state->parent = parent;
    --state->level;
    return state;
}

static void * gc_copy(void * context, char * (*whereTo)(void * context, size_t size), bool (*shouldCopy)(const void * context, const char * p), const char * from) {
    const char * to = from;
    if ((*shouldCopy)(context, from)) {
        to = gc_copy_nonref(context, whereTo, from);
    } else {
        // return (void *) to;
    }
    copy_state_t state = {to, whereTo, shouldCopy, context, 1};
    glc_box_walkobj(&state, &gc_copy_reference, from);
    return (void *) to;
}
