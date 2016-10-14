#ifndef SYNC_H
#define SYNC_H

#include "types.h"

__BEGIN_DECLS

#ifdef __GNUC__

inline int sync_cmpxchg_ptr(void * volatile *ptr, void *val, void *old) {
    return __sync_bool_compare_and_swap(ptr, old, val);
}

inline int sync_cmpxchg_i32(int32_t volatile *ptr, int32_t val, int32_t old) {
    return __sync_bool_compare_and_swap(ptr, old, val);
}

inline int sync_cmpxchg_u32(uint32_t volatile *ptr, uint32_t val, uint32_t old) {
    return __sync_bool_compare_and_swap(ptr, old, val);
}

inline int sync_cmpxchg_u64(uint64_t volatile *ptr, uint64_t val, uint64_t old) {
    return __sync_bool_compare_and_swap(ptr, old, val);
}

inline void *sync_xchg_ptr(void * volatile *ptr, void *val) {
    return (void *)__sync_lock_test_and_set(ptr, val);
}

inline int32_t sync_add_i32(int32_t volatile *ptr, int32_t val) {
    return __sync_add_and_fetch(ptr, val);
}

inline uint32_t sync_add_u32(uint32_t volatile *ptr, uint32_t val) {
    return __sync_add_and_fetch(ptr, val);
}

inline uint64_t sync_add_u64(uint64_t volatile *ptr, uint64_t val) {
    return __sync_add_and_fetch(ptr, val);
}

#endif

void *sync_lock_ptr(void * volatile *);
void sync_unlock_ptr(void * volatile *, void *);

__END_DECLS

#endif
