#include "sync.h"

#define SYNC_MARKER ((void *)-1)

void *sync_lock_ptr(void *volatile *loc)
{
    void *ptr;
    if (loc) {
        do {
            ptr = *loc;
            if (ptr != SYNC_MARKER) {
                if (sync_cmpxchg_ptr(loc, SYNC_MARKER, ptr)) {
                    break;
                }
            }
        } while (1);
    }
    else {
        ptr = NULL;
    }
    return ptr;
}

void sync_unlock_ptr(void *volatile *loc, void *ptr)
{
    if (loc) {
        while (!sync_cmpxchg_ptr(loc, ptr, SYNC_MARKER)) {
            _mm_pause();
        }
    }
}
