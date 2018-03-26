#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include "error.h"

#if defined(__cplusplus)

#pragma mark - Vector 2 Declaration
template <typename T>
struct Vector2 {
    Vector2() { x = 0; y = 0; }
    Vector2(T x, T y) : x(x), y(y) { }

    T          length() const;
    T          length_sq() const;

    Vector2<T>   operator +  (const Vector2<T> &v) const;
    Vector2<T> & operator += (const Vector2<T> &v);
    Vector2<T>   operator -  () const;
    Vector2<T>   operator -  (const Vector2<T> &v) const;
    Vector2<T> & operator -= (const Vector2<T> &v);
    Vector2<T>   operator *  (T f) const;
    Vector2<T>   operator *  (const Vector2<T> &v) const;
    Vector2<T> & operator *= (T f);
    Vector2<T> & operator *= (const Vector2<T> &v);
    Vector2<T>   operator /  (T f) const;
    Vector2<T> & operator /= (T f);
    T            operator [] (size_t i) const;
    T          & operator [] (size_t i);

    T x, y;
};

typedef Vector2<float> v2f;
typedef Vector2<double> v2d;

template <typename T>
T Vector2<T>::length() const
{
    return std::sqrt(length_sq());
}

template <typename T>
T Vector2<T>::length_sq() const
{
    return x * x + y * y;
}

template <typename T>
Vector2<T> Vector2<T>::operator + (const Vector2<T> &v) const
{
    return { x + v.x, y + v.y };
}

template <typename T>
Vector2<T> & Vector2<T>::operator += (const Vector2<T> &v)
{
    x += v.x;
    y += v.y;

    return *this;
}

template <typename T>
Vector2<T> Vector2<T>::operator - () const
{
    return { -x, -y };
}

template <typename T>
Vector2<T> Vector2<T>::operator - (const Vector2<T> &v) const
{
    return { x - v.x, y - v.y };
}

template <typename T>
Vector2<T> & Vector2<T>::operator -= (const Vector2<T> &v)
{
    x -= v.x;
    y -= v.y;

    return *this;
}

template <typename T>
Vector2<T> Vector2<T>::operator * (T f) const
{
    return { x * f, y * f };
}

template <typename T>
Vector2<T> Vector2<T>::operator * (const Vector2<T> &v) const
{
    return { x * v.x, y * v.y };
}

template <typename T>
Vector2<T> & Vector2<T>::operator *= (T f)
{
    x *= f;
    y *= f;

    return *this;
}

template <typename T>
Vector2<T> & Vector2<T>::operator *= (const Vector2<T> &v)
{
    x *= v.x;
    y *= v.y;

    return *this;
}

template <typename T>
Vector2<T> Vector2<T>::operator / (T f) const
{
    T inv = 1 / f;
    return { x * inv, y * inv };
}

template <typename T>
Vector2<T> & Vector2<T>::operator /= (T f)
{
    T inv = 1 / f;
    x *= inv;
    y *= inv;

    return *this;
}

template <typename T>
T Vector2<T>::operator [] (size_t i) const
{
    ASSERT(i < 2);
    return i == 0 ? x : y;
}

template <typename T>
T & Vector2<T>::operator [] (size_t i)
{
    ASSERT(i < 2);
    return i == 0 ? x : y;
}

#pragma mark Inline Functions

template <typename T, typename U>
inline Vector2<T> operator * (U s, const Vector2<T> &v) {
    return v * s;
}

template <typename T>
inline T Dot(const Vector2<T> &va, const Vector2<T> &vb)
{
    return va.x * vb.x + va.y * vb.y;
}

template <typename T>
inline Vector2<T> Lerp(T t, const Vector2<T> &va, const Vector2<T> &vb)
{
    return (1 - t) * va + t * vb;
}

template <typename T>
inline Vector2<T> Normalize(const Vector2<T> &v)
{
    return v / v.length();
}

template <typename T>
inline Vector2<T> Reflect(const Vector2<T> &v, const Vector2<T> &n)
{
    return v - 2 * Dot(v, n) * n;
}

#pragma mark - Vector 3 Declaration

template <typename T>
struct Vector3 {
    Vector3() { x = 0; y = 0; z = 0; }
    Vector3(T x, T y, T z) : x(x), y(y), z(z) { }

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
    T            operator [] (size_t i) const;
    T          & operator [] (size_t i);

    T x, y, z;
};

typedef Vector3<float> v3f;
typedef Vector3<double> v3d;

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

template <typename T>
T Vector3<T>::operator [] (size_t i) const
{
    ASSERT(i < 3);
    return i == 0 ? x : (i == 1 ? y : z);
}

template <typename T>
T & Vector3<T>::operator [] (size_t i)
{
    ASSERT(i < 3);
    return i == 0 ? x : (i == 1 ? y : z);
}

#pragma mark Inline Functions

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
inline Vector3<T> Normalize(const Vector3<T> &v)
{
    return v / v.length();
}

template <typename T>
inline Vector3<T> Reflect(const Vector3<T> &v, const Vector3<T> &n)
{
    return v - 2 * Dot(v, n) * n;
}


#pragma mark - Bounds 3 Declaration

template <typename T>
 struct Bounds3 {
    Bounds3() {
        T min = std::numeric_limits<T>::lowest();
        T max = std::numeric_limits<T>::max();

        lo = Vector3<T>(max, max, max);
        hi = Vector3<T>(min, min, min);
    }
    Bounds3(const Vector3<T> &p) : lo(p), hi(p) { }
    Bounds3(const Vector3<T> &p1,
            const Vector3<T> &p2) : lo(std::min(p1.x, p2.x),
                                       std::min(p1.y, p2.y),
                                       std::min(p1.z, p2.z)),
                                    hi(std::max(p1.x, p2.x),
                                       std::max(p1.y, p2.y),
                                       std::max(p1.z, p2.z)) { }

    Vector3<T> diagonal() const;
    T          area() const;
    T          volume() const;

    Vector3<T> lo;
    Vector3<T> hi;
};

typedef Bounds3<float> bounds3f;

template <typename T>
Vector3<T> Bounds3<T>::diagonal() const
{
    return hi - lo;
}

template <typename T>
T Bounds3<T>::area() const
{
    Vector3<T> d = diagonal();
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

template <typename T>
T Bounds3<T>::volume() const
{
    Vector3<T> d = diagonal();
    return d.x * d.y * d.z;
}

#pragma mark Inline Functions;

template <typename T>
inline Bounds3<T> Union(const Bounds3<T> &b, const Vector3<T> &p)
{
    Vector3<T> lo = {
        std::min(b.lo.x, p.x),
        std::min(b.lo.y, p.y),
        std::min(b.lo.z, p.z),
    };
    Vector3<T> hi = {
        std::max(b.hi.x, p.x),
        std::max(b.hi.y, p.y),
        std::max(b.hi.z, p.z),
    };
    return Bounds3<T>(lo, hi);
}

template <typename T>
inline Bounds3<T> Union(const Bounds3<T> &b1, const Bounds3<T> &b2)
{
    Vector3<T> lo = {
        std::min(b1.lo.x, b2.lo.x),
        std::min(b1.lo.y, b2.lo.y),
        std::min(b1.lo.z, b2.lo.z),
    };
    Vector3<T> hi = {
        std::max(b1.hi.x, b2.hi.x),
        std::max(b1.hi.y, b2.hi.y),
        std::max(b1.hi.z, b2.hi.z),
    };
    return Bounds3<T>(lo, hi);
}

#else

typedef struct v2f {
    float x;
    float y;
} v2f;

typedef struct v2d {
    double x;
    double y;
} v2d;

typedef struct v3f {
    float x;
    float y;
    float z;
} v3f;

typedef struct v3d {
    double x;
    double y;
    double z;
} v3d;

typedef struct bounds3f {
    v3f lo;
    v3f hi;
} bounds3f;

#endif
