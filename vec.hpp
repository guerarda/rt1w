#ifndef VEC_H
#define VEC_H

#include <stdint.h>

__BEGIN_DECLS

typedef struct v3f {
    float x;
    float y;
    float z;
} v3f;

__END_DECLS

#if defined(__cplusplus)

v3f   v3f_add(const v3f &va, const v3f &vb);
v3f   v3f_sub(const v3f &va, const v3f &vb);
v3f   v3f_cross(const v3f &va, const v3f &vb);
float v3f_dot(const v3f &va, const v3f &vb);
v3f   v3f_lerp(float t, const v3f &va, const v3f &vb);
v3f   v3f_muls(float s, const v3f &v);
float v3f_norm(const v3f &v);
float v3f_norm_sq(const v3f &v);
v3f   v3f_normalize(const v3f &v);

#endif
#endif
