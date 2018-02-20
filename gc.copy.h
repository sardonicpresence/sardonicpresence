/**
 * Copy-collection functionality.
 */
#pragma once

#include "rt.h"
#include "box.h"

extern uint64_t g_copied;

typedef struct {
    char * parent;
    char * (*whereTo)(void *, size_t);
    bool (*shouldCopy)(const void *, const char *);
    void * context;
} copy_state_t;

static char * gc_copy_nonref(void * context, char * (*whereTo)(void * context, size_t size), const char * from) {
    copy_state_t * state = (copy_state_t *) context;
    size_t size = glc_box_sizeof(from);
    size_t toCopy = glc_box_copysize(from);
    char * to = (*state->whereTo)(state->context, size);
    memcpy(to, from, toCopy);
    g_copied += toCopy;
    return to;
}

static void * gc_copy_reference(void * context, const char * from, size_t offset) {
    copy_state_t * state = (copy_state_t *) context;
    if (!(*state->shouldCopy)(state->context, from))
        return state;

    // TODO: Consider tagging/untagging

    char * to = gc_copy_nonref(state->context, state->whereTo, from);

    // Copy referenced objects, updating references
    char * parent = state->parent;
    *((char **) (parent + offset)) = to;
    state->parent = to;
    glc_box_walkobj(state, &gc_copy_reference, from);
    state->parent = parent;
    return state;
}

static void * gc_copy(void * context, char * (*whereTo)(void * context, size_t size), bool (*shouldCopy)(const void * context, const char * p), const char * from) {
    char * to = gc_copy_nonref(context, whereTo, from);
    copy_state_t state = {to, whereTo, shouldCopy, context};
    glc_box_walkobj(&state, &gc_copy_reference, from);
    return to;
}
