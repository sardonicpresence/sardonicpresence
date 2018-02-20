#include <stdint.h>

static uint64_t x = 0, w = 0, s = 0xb5ad4eceda1ce2a9;

inline static uint32_t random() {
   x *= x;
   x += (w += s);
   return x = (x>>32) | (x<<32);
}
