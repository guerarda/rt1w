#pragma once

#include "error.h"

#ifdef __cplusplus

#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#pragma mark - Vector 2 Declaration

template <typename T>
struct Vector2 {
    static_assert(std::is_arithmetic<T>(), "Vector2 with non arithmetic type");

    Vector2() = default;
    Vector2(T x, T y) : x(x), y(y) {}

    T length() const;
    T length_sq() const;

    Vector2<T> operator+(const Vector2<T> &v) const;
    Vector2<T> &operator+=(const Vector2<T> &v);
    Vector2<T> operator-() const;
    Vector2<T> operator-(const Vector2<T> &v) const;
    Vector2<T> &operator-=(const Vector2<T> &v);
    Vector2<T> operator*(T f) const;
    Vector2<T> operator*(const Vector2<T> &v) const;
    Vector2<T> &operator*=(T f);
    Vector2<T> &operator*=(const Vector2<T> &v);
    Vector2<T> operator/(T f) const;
    Vector2<T> &operator/=(T f);
    T operator[](size_t i) const;
    T &operator[](size_t i);

    T x = 0;
    T y = 0;
};

typedef Vector2<int32_t> v2i;
typedef Vector2<uint32_t> v2u;
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
Vector2<T> Vector2<T>::operator+(const Vector2<T> &v) const
{
    return { x + v.x, y + v.y };
}

template <typename T>
Vector2<T> &Vector2<T>::operator+=(const Vector2<T> &v)
{
    x += v.x;
    y += v.y;

    return *this;
}

template <typename T>
Vector2<T> Vector2<T>::operator-() const
{
    return { -x, -y };
}

template <typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T> &v) const
{
    return { x - v.x, y - v.y };
}

template <typename T>
Vector2<T> &Vector2<T>::operator-=(const Vector2<T> &v)
{
    x -= v.x;
    y -= v.y;

    return *this;
}

template <typename T>
Vector2<T> Vector2<T>::operator*(T f) const
{
    return { x * f, y * f };
}

template <typename T>
Vector2<T> Vector2<T>::operator*(const Vector2<T> &v) const
{
    return { x * v.x, y * v.y };
}

template <typename T>
Vector2<T> &Vector2<T>::operator*=(T f)
{
    x *= f;
    y *= f;

    return *this;
}

template <typename T>
Vector2<T> &Vector2<T>::operator*=(const Vector2<T> &v)
{
    x *= v.x;
    y *= v.y;

    return *this;
}

template <typename T>
Vector2<T> Vector2<T>::operator/(T f) const
{
    T inv = 1 / f;
    return { x * inv, y * inv };
}

template <typename T>
Vector2<T> &Vector2<T>::operator/=(T f)
{
    T inv = 1 / f;
    x *= inv;
    y *= inv;

    return *this;
}

template <typename T>
T Vector2<T>::operator[](size_t i) const
{
    ASSERT(i < 2);
    return i == 0 ? x : y;
}

template <typename T>
T &Vector2<T>::operator[](size_t i)
{
    ASSERT(i < 2);
    return i == 0 ? x : y;
}

#pragma mark Inline Functions

template <typename T, typename U>
inline Vector2<T> operator*(U s, const Vector2<T> &v)
{
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

template <typename T>
inline T Distance(const Vector2<T> &va, const Vector2<T> &vb)
{
    return (va - vb).length();
}

template <typename T>
inline T DistanceSquared(const Vector2<T> &va, const Vector2<T> &vb)
{
    return (va - vb).length_sq();
}

#pragma mark - Vector 3 Declaration

template <typename T>
struct Vector3 {
    static_assert(std::is_arithmetic<T>(), "Vector3 with non arithmetic type");

    Vector3() = default;
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

    T length() const;
    T length_sq() const;

    Vector3<T> operator+(const Vector3<T> &v) const;
    Vector3<T> &operator+=(const Vector3<T> &v);
    Vector3<T> operator-() const;
    Vector3<T> operator-(const Vector3<T> &v) const;
    Vector3<T> &operator-=(const Vector3<T> &v);
    Vector3<T> operator*(T f) const;
    Vector3<T> operator*(const Vector3<T> &v) const;
    Vector3<T> &operator*=(T f);
    Vector3<T> &operator*=(const Vector3<T> &v);
    Vector3<T> operator/(T f) const;
    Vector3<T> &operator/=(T f);
    Vector3<T> operator/(const Vector3<T> &v) const;
    Vector3<T> &operator/(const Vector3<T> &v);
    T operator[](size_t i) const;
    T &operator[](size_t i);

    T x = 0;
    T y = 0;
    T z = 0;
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
Vector3<T> Vector3<T>::operator+(const Vector3<T> &v) const
{
    return { x + v.x, y + v.y, z + v.z };
}

template <typename T>
Vector3<T> &Vector3<T>::operator+=(const Vector3<T> &v)
{
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator-() const
{
    return { -x, -y, -z };
}

template <typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T> &v) const
{
    return { x - v.x, y - v.y, z - v.z };
}

template <typename T>
Vector3<T> &Vector3<T>::operator-=(const Vector3<T> &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator*(T f) const
{
    return { x * f, y * f, z * f };
}

template <typename T>
Vector3<T> Vector3<T>::operator*(const Vector3<T> &v) const
{
    return { x * v.x, y * v.y, z * v.z };
}

template <typename T>
Vector3<T> &Vector3<T>::operator*=(T f)
{
    x *= f;
    y *= f;
    z *= f;

    return *this;
}

template <typename T>
Vector3<T> &Vector3<T>::operator*=(const Vector3<T> &v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator/(T f) const
{
    T inv = 1 / f;
    return { x * inv, y * inv, z * inv };
}

template <typename T>
Vector3<T> &Vector3<T>::operator/=(T f)
{
    T inv = 1 / f;
    x *= inv;
    y *= inv;
    z *= inv;

    return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator/(const Vector3<T> &v) const
{
    ASSERT(v.x)
    return { x / v.x, y / v.y, z / v.z };
}

template <typename T>
Vector3<T> &Vector3<T>::operator/(const Vector3<T> &v)
{
    x /= v.x;
    y /= v.y;
    z /= v.z;

    return *this;
}

template <typename T>
T Vector3<T>::operator[](size_t i) const
{
    ASSERT(i < 3);
    return i == 0 ? x : (i == 1 ? y : z);
}

template <typename T>
T &Vector3<T>::operator[](size_t i)
{
    ASSERT(i < 3);
    return i == 0 ? x : (i == 1 ? y : z);
}

#pragma mark Inline Functions

template <typename T, typename U>
inline Vector3<T> operator*(U s, const Vector3<T> &v)
{
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
    return { va.y * vb.z - va.z * vb.y,    //
             va.z * vb.x - va.x * vb.z,    //
             va.x * vb.y - va.y * vb.x };
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

template <typename T>
inline T Distance(const Vector3<T> &va, const Vector3<T> &vb)
{
    return (va - vb).length();
}

template <typename T>
inline T DistanceSquared(const Vector3<T> &va, const Vector3<T> &vb)
{
    return (va - vb).length_sq();
}

#pragma mark - Vector 4 Declaration

template <typename T>
struct Vector4 {
    static_assert(std::is_arithmetic<T>(), "Vector4 with non arithmetic type");

    Vector4() = default;
    Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;
};

typedef Vector4<float> v4f;
typedef Vector4<double> v4d;

#pragma mark - Matrix 4x4 Declaration

template <typename T>
struct Matrix4x4 {
    static_assert(std::is_floating_point<T>(), "Matrix44 with non floating point type");

    Matrix4x4() = default;
    Matrix4x4(const Vector4<T> &vx,
              const Vector4<T> &vy,
              const Vector4<T> &vz,
              const Vector4<T> &vw) :
        vx(vx),
        vy(vy),
        vz(vz),
        vw(vw)
    {}
    Matrix4x4(T m[4][4])
    {
        vx = { m[0][0], m[0][1], m[0][2], m[0][3] };
        vy = { m[1][0], m[1][1], m[1][2], m[1][3] };
        vz = { m[2][0], m[2][1], m[2][2], m[2][3] };
        vw = { m[3][0], m[3][1], m[3][2], m[3][3] };
    }

    Vector4<T> vx, vy, vz, vw;
};

typedef Matrix4x4<float> m44f;
typedef Matrix4x4<double> m44d;

#pragma mark Inline Functions

template <typename T>
inline Matrix4x4<T> Identity4x4()
{
    T m[4][4] = { { 1.0, 0.0, 0.0, 0.0 },
                  { 0.0, 1.0, 0.0, 0.0 },
                  { 0.0, 0.0, 1.0, 0.0 },
                  { 0.0, 0.0, 0.0, 1.0 } };
    return Matrix4x4<T>(m);
}

template <typename T>
inline Matrix4x4<T> Inverse(const Matrix4x4<T> &m)
{
    size_t indxc[4], indxr[4];
    size_t ipiv[4] = { 0, 0, 0, 0 };
    T minv[4][4];
    memcpy(minv, &m.vx.x, 4 * 4 * sizeof(T));
    for (int i = 0; i < 4; i++) {
        size_t irow = 0, icol = 0;
        double big = 0;
        // Choose pivot
        for (size_t j = 0; j < 4; j++) {
            if (ipiv[j] != 1) {
                for (size_t k = 0; k < 4; k++) {
                    if (ipiv[k] == 0) {
                        if ((double)std::abs(minv[j][k]) >= big) {
                            big = double(std::abs(minv[j][k]));
                            irow = j;
                            icol = k;
                        }
                    }
                    else {
                        ERROR_IF(ipiv[k] > 1, "Singular matrix in MatrixInvert");
                    }
                }
            }
        }
        ++ipiv[icol];
        // Swap rows _irow_ and _icol_ for pivot
        if (irow != icol) {
            for (int k = 0; k < 4; ++k) {
                std::swap(minv[irow][k], minv[icol][k]);
            }
        }
        indxr[i] = irow;
        indxc[i] = icol;
        ERROR_IF(FloatEqual(minv[icol][icol], T{ 0.0 }),
                 "Singular matrix in MatrixInvert");

        // Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
        T pivinv = 1 / minv[icol][icol];
        minv[icol][icol] = 1.;
        for (size_t j = 0; j < 4; j++) {
            minv[icol][j] *= pivinv;
        }

        // Subtract this row from others to zero out their columns
        for (size_t j = 0; j < 4; j++) {
            if (j != icol) {
                T save = minv[j][icol];
                minv[j][icol] = 0;
                for (size_t k = 0; k < 4; k++) {
                    minv[j][k] -= minv[icol][k] * save;
                }
            }
        }
    }
    // Swap columns to reflect permutation
    for (size_t j = 3; j--;) {
        if (indxr[j] != indxc[j]) {
            for (size_t k = 0; k < 4; k++) {
                std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
            }
        }
    }
    return Matrix4x4<T>(minv);
}

template <typename T>
inline Matrix4x4<T> Transpose(const Matrix4x4<T> &m)
{
    T t[4][4] = { { m.vx.x, m.vy.x, m.vz.x, m.vw.x },
                  { m.vx.y, m.vy.y, m.vz.y, m.vw.y },
                  { m.vx.z, m.vy.z, m.vz.z, m.vw.z },
                  { m.vx.w, m.vy.w, m.vz.w, m.vw.w } };
    return Matrix4x4<T>(t);
}

template <typename T>
inline Matrix4x4<T> Mul(const Matrix4x4<T> &ma, const Matrix4x4<T> &mb)
{
    Matrix4x4<T> m;
    const T *pa = &ma.vx.x;
    const T *pb = &mb.vx.x;
    T *pm = &m.vx.x;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            pm[i * 4 + j] = pa[i * 4 + 0] * pb[0 * 4 + j]
                            + pa[i * 4 + 1] * pb[1 * 4 + j]    //
                            + pa[i * 4 + 2] * pb[2 * 4 + j]    //
                            + pa[i * 4 + 3] * pb[3 * 4 + j];
        }
    }
    return m;
}

inline m44f m44f_identity()
{
    return Identity4x4<float>();
}

inline m44d m44d_identity()
{
    return Identity4x4<double>();
}

#pragma mark - Bounds 3 Declaration

template <typename T>
struct Bounds3 {
    static_assert(std::is_arithmetic<T>(), "Bounds3 with non arithmetic type");

    Bounds3()
    {
        T min = std::numeric_limits<T>::lowest();
        T max = std::numeric_limits<T>::max();

        lo = Vector3<T>(max, max, max);
        hi = Vector3<T>(min, min, min);
    }
    Bounds3(const Vector3<T> &p) : lo(p), hi(p) {}
    Bounds3(const Vector3<T> &p1, const Vector3<T> &p2) :
        lo(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
        hi(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
    {}

    Vector3<T> center() const;
    Vector3<T> diagonal() const;
    T area() const;
    T volume() const;
    int32_t maxAxis() const;

    Vector3<T> lo;
    Vector3<T> hi;
};

typedef Bounds3<float> bounds3f;

template <typename T>
Vector3<T> Bounds3<T>::center() const
{
    return T(0.5) * (lo + hi);
}

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

template <typename T>
int32_t Bounds3<T>::maxAxis() const
{
    Vector3<T> d = hi - lo;
    return d.x > d.y ? (d.x > d.z ? 0 : 2) : (d.y > d.z ? 1 : 2);
}

#pragma mark Inline Functions

template <typename T>
inline Vector3<T> Offset(const Bounds3<T> &b, const Vector3<T> &p)
{
    return (p - b.lo) / (b.hi - b.lo);
}

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

typedef struct rectf {
    v2f org;
    v2f size;
} rectf;

#else

#include <math.h>

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

typedef struct v4f {
    float x;
    float y;
    float z;
    float w;
};

typedef struct v4d {
    double x;
    double y;
    double z;
    double w;
};

typedef struct m44f {
    v4f vx;
    v4f vy;
    v4f vz;
    v4f vw;
};

typedef struct m44d {
    v4d vx;
    v4d vy;
    v4d vz;
    v4d vw;
};

typedef struct bounds3f {
    v3f lo;
    v3f hi;
} bounds3f;

#endif
