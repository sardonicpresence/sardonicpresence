#include <stdint.h>

#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RW 4

__attribute__((dllimport)) __stdcall
extern void * VirtualAlloc(void * p, size_t size, long type, long protect);
