#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#endif

#include <x86intrin.h>

__BEGIN_DECLS

typedef struct rect {
    struct {
        int32_t x;
        int32_t y;
    } org;
    struct {
        uint32_t x;
        uint32_t y;
    } size;
} rect_t;

typedef enum {
    TYPE_VOID = 0x0,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT32,
    TYPE_FLOAT64,
} buffer_type_t;

typedef enum {
    ORDER_RGBA,
    ORDER_ARGB,
    ORDER_BGRA,
    ORDER_R,
    ORDER_RG,
    ORDER_RGB
} buffer_order_t;

typedef struct buffer_format {
    uint16_t size;
    buffer_type_t type : 8;
    buffer_order_t order : 8;
} buffer_format_t;

typedef struct buffer {
    void *data;
    size_t bpr;
    rect_t rect;
    buffer_format_t format;
} buffer_t;

inline size_t buffer_order_sizeof(buffer_order_t t)
{
    switch (t) {
    case ORDER_R: return 1;

    case ORDER_RG: return 2;

    case ORDER_RGB: return 3;

    case ORDER_RGBA:
    case ORDER_ARGB:
    case ORDER_BGRA: return 4;

    default: return 0;
    }
}

inline size_t buffer_type_sizeof(buffer_type_t t)
{
    switch (t) {
    case TYPE_INT8:
    case TYPE_UINT8: return 1;

    case TYPE_INT16:
    case TYPE_UINT16: return 2;

    case TYPE_INT32:
    case TYPE_UINT32:
    case TYPE_FLOAT32: return 4;

    case TYPE_INT64:
    case TYPE_UINT64:
    case TYPE_FLOAT64: return 8;

    default: return 0;
    }
}

inline buffer_format_t buffer_format_init(buffer_type_t t, buffer_order_t o)
{
    uint16_t s = (uint16_t)(buffer_type_sizeof(t) * buffer_order_sizeof(o));
    buffer_format_t f = { s, t, o };
    return f;
}

__END_DECLS
