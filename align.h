/**
 * Utilities related to pointer alignment.
 */

#pragma once

/**
 * Return the next pointer, equal to or greater than the given pointer,
 * with the given alignment.
 */
static char * align(char * p, size_t align) {
    return (char *) ((size_t) (p + align - 1) & -align);
}

/**
 * Return the next pointer, equal to or greater than the given pointer,
 * suitably aligned to store an object of the given size in bytes.
 */
static char * alignSize(char * p, size_t size) {
    if (size < 2)
        return p;
    if (size >= 8)
        return align(p, 8);
    return align(p, 1 + ((size >> 1) | (size >> 2)));
}
