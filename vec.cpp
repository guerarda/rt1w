#include "vec.hpp"
#include <assert.h>
#include <math.h>

v3f v3f_add(const v3f &va, const v3f &vb)
{
    v3f v = {
        va.x + vb.x,
        va.y + vb.y,
        va.z + vb.z
    };
    return v;
}

v3f v3f_sub(const v3f &va, const v3f &vb)
{
    v3f v = {
        va.x - vb.x,
        va.y - vb.y,
        va.z - vb.z
    };
    return v;
}

v3f v3f_cross(const v3f &va, const v3f &vb)
{
    v3f v = {
        va.y * vb.z - va.z * vb.y,
        va.z * vb.x - va.x * vb.z,
        va.x * vb.y - va.y * vb.x
    };
    return v;
}

float v3f_dot(const v3f &va, const v3f &vb)
{
    return va.x * vb.x + va.y * vb.y + va.z * vb.z;
}

v3f v3f_lerp(float t, const v3f &va, const v3f &vb)
{
    v3f v = v3f_smul(1.0f - t, va);
    v3f u = v3f_smul(t, vb);

    return v3f_add(v, u);
}

v3f v3f_smul(float s, const v3f &v)
{
    v3f u = {
        s * v.x,
        s * v.y,
        s * v.z,
    };
    return u;
}

v3f v3f_vmul(const v3f &va, const v3f &vb)
{
    v3f v = {
        va.x * vb.x,
        va.y * vb.y,
        va.z * vb.z
    };
    return v;
}

float v3f_norm_sq(const v3f &v)
{
    return v.x * v.x + v.y * v.y + v.z* v.z;
}

float v3f_norm(const v3f &v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z* v.z);
}

v3f v3f_normalize(const v3f &v)
{
    float n = v3f_norm(v);
    v3f u = {
        v.x / n,
        v.y / n,
        v.z / n
    };
    return u;
}
