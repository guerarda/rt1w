#ifndef VEC_H
#define VEC_H

#include "types.h"

__BEGIN_DECLS


__END_DECLS

#if defined(__cplusplus)

v3f   v3f_add(const v3f &va, const v3f &vb);
v3f   v3f_sub(const v3f &va, const v3f &vb);
v3f   v3f_smul(float s, const v3f &v);
v3f   v3f_vmul(const v3f &va, const v3f &vb);
v3f   v3f_cross(const v3f &va, const v3f &vb);
float v3f_dot(const v3f &va, const v3f &vb);
v3f   v3f_lerp(float t, const v3f &va, const v3f &vb);
float v3f_norm(const v3f &v);
float v3f_norm_sq(const v3f &v);
v3f   v3f_normalize(const v3f &v);
v3f   v3f_reflect(const v3f &v, const v3f &n);

#endif
#endif
