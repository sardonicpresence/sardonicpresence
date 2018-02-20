void __chkstk() {}

void * memcpy(void * restrict to, const void * restrict from, size_t bytes) {
    for (size_t i = 0; i < bytes; ++i) {
        ((char *) to)[i] = ((char *) from)[i];
    }
    return to;
}

void * memset(void * restrict to, int v, size_t bytes) {
    for (size_t i = 0; i < bytes; ++i) {
        ((char *) to)[i] = v;
    }
    return to;
}
