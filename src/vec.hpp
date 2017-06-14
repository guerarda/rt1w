#pragma once

#include "types.h"
#include <cmath>

#if defined(__cplusplus)

#pragma mark - Vector 3 Declaration

template <typename T>
struct Vector3 {
    Vector3() { x = 0; y = 0; z = 0; }
    Vector3(T x, T y, T z) : x(x), y(y), z(z) { }

    Vector3<T> normalized() const;
    T          length() const;
    T          length_sq() const;

    Vector3<T>   operator +  (const Vector3<T> &v) const;
    Vector3<T> & operator += (const Vector3<T> &v);
    Vector3<T>   operator -  () const;
    Vector3<T>   operator -  (const Vector3<T> &v) const;
    Vector3<T> & operator -= (const Vector3<T> &v);
    Vector3<T>   operator *  (T f) const;
    Vector3<T>   operator *  (const Vector3<T> &v) const;
    Vector3<T> & operator *= (T f);
    Vector3<T> & operator *= (const Vector3<T> &v);
    Vector3<T>   operator /  (T f) const;
    Vector3<T> & operator /= (T f);

    T x, y, z;
};
typedef Vector3<float> v3f;

template <typename T>
Vector3<T> Vector3<T>::normalized() const
{
    return *this / length();
}

template <typename T>
T Vector3<T>::length() const
{
    return std::sqrt(length_sq());
}

template <typename T>
T Vector3<T>::length_sq() const
{
    return x * x + y * y + z * z;
}

template <typename T>
Vector3<T> Vector3<T>::operator + (const Vector3<T> &v) const
{
    return { x + v.x, y + v.y, z + v.z };
}

template <typename T>
Vector3<T> & Vector3<T>::operator += (const Vector3<T> &v)
{
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator - () const
{
    return { -x, -y, -z };
}

template <typename T>
Vector3<T> Vector3<T>::operator - (const Vector3<T> &v) const
{
    return { x - v.x, y - v.y, z - v.z };
}

template <typename T>
Vector3<T> & Vector3<T>::operator -= (const Vector3<T> &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator * (T f) const
{
    return { x * f, y * f, z * f };
}

template <typename T>
Vector3<T> Vector3<T>::operator * (const Vector3<T> &v) const
{
    return { x * v.x, y * v.y, z * v.z };
}

template <typename T>
Vector3<T> & Vector3<T>::operator *= (T f)
{
    x *= f;
    y *= f;
    z *= f;

    return *this;
}

template <typename T>
Vector3<T> & Vector3<T>::operator *= (const Vector3<T> &v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator / (T f) const
{
    T inv = 1 / f;
    return { x * inv, y * inv, z * inv };
}

template <typename T>
Vector3<T> & Vector3<T>::operator /= (T f)
{
    T inv = 1 / f;
    x *= inv;
    y *= inv;
    z *= inv;

    return *this;
}

#pragma mark Inline functions

template <typename T, typename U>
inline Vector3<T> operator * (U s, const Vector3<T> &v) {
    return v * s;
}

template <typename T>
inline T Dot(const Vector3<T> &va, const Vector3<T> &vb)
{
    return va.x * vb.x + va.y * vb.y + va.z * vb.z;
}

template <typename T>
inline Vector3<T> Cross(const Vector3<T> &va, const Vector3<T> &vb)
{
    return {
        va.y * vb.z - va.z * vb.y,
        va.z * vb.x - va.x * vb.z,
        va.x * vb.y - va.y * vb.x
    };
}

template <typename T>
inline Vector3<T> Lerp(T t, const Vector3<T> &va, const Vector3<T> &vb)
{
    return (1 - t) * va + t * vb;
}

template <typename T>
inline Vector3<T> Reflect(const Vector3<T> &v, const Vector3<T> &n)
{
    return v - 2 * Dot(v, n) * n;
}

typedef struct box {
    v3f lo;
    v3f hi;
} box_t;

#else

typedef struct v3f {
    float x;
    float y;
    float z;
} v3f;

#endif
