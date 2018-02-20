/**
 * Types and functions related to how values are stored in the managed heap.
 */

#pragma once

typedef struct {
    uint32_t nother; // Number of qwords in addition to the references
    uint32_t nref; // Number of references to managed objects that follow
    // Total size of object is 64*(1+nother+nref) on 64-bit
    // Above counts could likely be 16-bit esp. on a 32-bit system
    const char * content[];
} box_t;

/**
 * The size in bytes of the object at the given pointer.
 */
static size_t glc_box_sizeof(const char * p) {
    box_t * box = (box_t *) p;
    return sizeof(box_t) + sizeof(char *) * box->nother + sizeof(char *) * box->nref;
}

/**
 * Number of leading bytes to copy when relocating the object at the given pointer.
 */
static size_t glc_box_copysize(const char * p) {
    box_t * box = (box_t *) p;
    return sizeof(box_t) + sizeof(char *) * box->nother;
}

/**
 * Iterate over all references within the object at the given pointer, calling the given callback
 * with the given context and a pointer to a reference, thread the new returned context through
 * to the next call-back.
 */
static void glc_box_walkobj(void * context, void * (*callback)(void * context, const char * ref, size_t offset), const char * p) {
    box_t * box = (box_t *) p;
    const char ** refs = &box->content[0];
    for (int i = 0; i < box->nref; ++i) {
        const char ** ref = refs + i;
        size_t offset = (char *) ref - (char *) box;
        context = (*callback)(context, *ref, offset);
    }
}
