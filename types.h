#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>
#include <stddef.h>

__BEGIN_DECLS

typedef struct v2i {
    int32_t x;
    int32_t y;
} v2i;

typedef struct v2u {
    uint32_t x;
    uint32_t y;
} v2u;

typedef struct v3f {
    float x;
    float y;
    float z;
} v3f;

typedef struct rect {
    v2i org;
    v2u size;
} rect;

__END_DECLS

#endif
