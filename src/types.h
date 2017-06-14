#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <x86intrin.h>

__BEGIN_DECLS

#define __zero_box { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }

typedef struct v2i {
    int32_t x;
    int32_t y;
} v2i;

typedef struct v2u {
    uint32_t x;
    uint32_t y;
} v2u;

typedef struct v2f {
    float x;
    float y;
} v2f;

typedef struct rect {
    v2i org;
    v2u size;
} rect_t;

typedef enum {
    TYPE_VOID,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT16,
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
    uint16_t       size;
    buffer_type_t  type:8;
    buffer_order_t order:8;
} buffer_format_t;

typedef struct buffer {
    void            *data;
    size_t           bpr;
    rect_t           rect;
    buffer_format_t  format;
} buffer_t;

inline size_t buffer_order_sizeof(buffer_order_t t)
{
    switch(t) {
    case ORDER_R:
        return 1;

    case ORDER_RG:
        return 2;

    case ORDER_RGB:
        return 3;

    case ORDER_RGBA:
    case ORDER_ARGB:
    case ORDER_BGRA:
        return 4;

    default:
        return 0;
    }
}

inline size_t buffer_type_sizeof(buffer_type_t t)
{
    switch (t) {
    case TYPE_INT8:
    case TYPE_UINT8:
        return 8;

    case TYPE_INT16:
    case TYPE_UINT16:
    case TYPE_FLOAT16:
        return 16;

    case TYPE_INT32:
    case TYPE_UINT32:
    case TYPE_FLOAT32:
        return 32;

    case TYPE_INT64:
    case TYPE_UINT64:
    case TYPE_FLOAT64:
        return 64;

    default:
        return 0;
    }
}

__END_DECLS

#endif
