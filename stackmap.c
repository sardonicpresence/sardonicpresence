#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#define MEM_COMMIT 4096
#define MEM_RESERVE 8192
#define MEM_READWRITE 4

__attribute__((alloc_size(2)))
void * __stdcall VirtualAlloc(const void * p, const size_t bytes, const int flags, const int access);

__attribute__((nothrow))
void * __os_alloc(const size_t bytes) {
    return VirtualAlloc(0, bytes, MEM_RESERVE | MEM_COMMIT, MEM_READWRITE);
}

struct header_t {
    uint8_t version;
    uint8_t _reserved1;
    uint16_t _reserved2;
};

struct stacksize_t {
    uint64_t pFunction;
    uint64_t stackSize;
    uint64_t nRecords;
};

struct stackmap_t {
    struct header_t header;
    uint32_t nFunctions;
    uint32_t nConstants;
    uint32_t nRecords;
    struct stacksize_t sizes[];
    // array of nConstants 64-bit constants
    // array of nRecords record_t
};

struct location_t {
    uint8_t location;
    uint8_t _reserved1;
    uint16_t bytes;
    uint16_t regnum;
    uint16_t _reserved2;
    int32_t offsetOrConstant;
};

struct liveout_t {
    uint16_t regnum;
    uint8_t _reserved;
    uint8_t bytes;
};

struct record_t {
    uint64_t id;
    uint32_t ipOffset;
    uint16_t flags;
    uint16_t nLocations;
    struct location_t locations[];
    // aligned to 8 Bytes
    // uint16_t _padding;
    // uint16_t nLiveOuts
    // array of liveouts, 32-bytes each
    // aligned to 8 bytes
};

struct record2_t {
    uint16_t nLiveout;
    struct liveout_t liveouts[];
};

static const void * alignedTo16(const void * p) {
    return (void *) ((((size_t) p + 7) / 8) * 8);
}

static const struct record_t * nextRecord(const struct record_t * record) {
    struct record2_t * liveout = (struct record2_t *) alignedTo16(record->locations + record->nLocations);
    return (struct record_t *) alignedTo16(liveout->liveouts + liveout->nLiveout);
}

__attribute__((nothrow))
void __stackmap_walk(const struct stackmap_t * pStackmap, bool (* callback)(void *, const void *, uint64_t, const struct record_t *), void * context) {
    const struct stacksize_t * sizes = pStackmap->sizes;
    const uint64_t * constants = (uint64_t *) (pStackmap->sizes + pStackmap->nFunctions);
    const struct record_t * record = (struct record_t *) (constants + pStackmap->nConstants);
    for (int i = 0; i < pStackmap->nFunctions; ++i) {
        for (int j = 0; j < sizes[i].nRecords; ++j) {
            const void * rip = (const void *) (sizes[i].pFunction + record->ipOffset);
            if (!(*callback)(context, rip, sizes[i].stackSize, record))
                return;
            record = nextRecord(record);
        }
    }
}

struct find_context_t {
    const void * rip;
    uint64_t stackSize;
    struct record_t const * record;
};

static bool __stackmap_find_callback(void * _context, const void * rip, uint64_t stackSize, const struct record_t * record) {
    struct find_context_t * context = _context;
    if (rip != context->rip)
        return true;
    context->stackSize = stackSize;
    context->record = record;
    return false;
}

const struct record_t * __stackmap_find(const struct stackmap_t * pStackmap, const void * rip, uint64_t * outStackSize) {
    struct find_context_t context;
    context.rip = rip;
    context.record = 0;
    __stackmap_walk(pStackmap, &__stackmap_find_callback, &context);
    *outStackSize = context.stackSize;
    return context.record;
}

static void ** __stackmap_location(const struct location_t * location, const char * rsp) {
    // assert (location->location == 3 && "Not an indirect reference location!");
    switch (location->location) {
        case 3: // Indirect [Reg + Offset]
            // assert (location->regnum == 7 && "Not RSP-relative!");
            return (void **) rsp + location->offsetOrConstant;
    }
    return 0;
}

static void __stack_walk(const struct stackmap_t * pStackmap, const char * rip, const char * rsp,
                         __attribute__((nothrow)) void * (* callback)(void *, void *), void * state)
{
    // __builtin_debugtrap();
    uint64_t stackSize;
    do {
        const struct record_t * record = __stackmap_find(pStackmap, rip, &stackSize);
        if (record == 0)
            return;
        printf("Stack-map record locations: %d\n", record->nLocations);
        for (int i = 3; i < record->nLocations - 1; i += 2) {
            const struct location_t * baseLocation = record->locations + i;
            const struct location_t * referenceLocation = record->locations + i + 1;
            void ** pbase = __stackmap_location(baseLocation, rsp);
            void ** preference = __stackmap_location(referenceLocation, rsp);
            void * prelocated = (*callback) (state, *pbase);
            *preference = ((char *) *preference - (char *) *pbase) + (char *) prelocated;
        }
        rsp += stackSize + 0x20; // Shadow space
        rip = *((void **) rsp + 1);
    } while (rip != 0);
}

extern void __stackmap_start__;

__attribute__((nothrow))
void gc(void * (* callback)(void *, void *), void * state) {
    const void * rip = __builtin_return_address(0);
    const char * bp = __builtin_frame_address(0);
    const char * sp = bp + 8; // Skip %rbp pushed onto the stack
    const void * rsp = sp + 0x28; // Spilled registers
    __stack_walk(&__stackmap_start__, rip, rsp, callback, state);
}
