#include "transform.hpp"

#include "error.h"

#include <cmath>

Transform Transform::operator*(const Transform &t) const
{
    return Transform(Mul(m_mat, t.mat()), Mul(t.inv(), m_inv));
}

Ray Transform::operator()(const Ray &r) const
{
    v3f o = Mulp(*this, r.org());
    v3f d = Mulv(*this, r.dir());

    return Ray(o, d);
}

Ray Transform::operator()(const Ray &r, v3f &oError, v3f &dError) const
{
    v3f o = Mulp(*this, r.org(), oError);
    v3f d = Mulv(*this, r.dir(), dError);

    return Ray(o, d);
}

Ray Transform::operator()(const Ray &r,
                          const v3f &oErrorIn,
                          const v3f &dErrorIn,
                          v3f &oErrorOut,
                          v3f &dErrorOut) const
{
    v3f o = Mulp(*this, r.org(), oErrorIn, oErrorOut);
    v3f d = Mulv(*this, r.dir(), dErrorIn, dErrorOut);

    return Ray(o, d);
}

Transform Inverse(const Transform &t)
{
    return Transform(t.inv(), t.mat());
}

template <typename T>
Vector3<T> Mulv(const Transform &t, const Vector3<T> &v)
{
    T x = v.x, y = v.y, z = v.z;
    m44f mat = t.mat();

    return { mat.vx.x * x + mat.vx.y * y + mat.vx.z * z,
             mat.vy.x * x + mat.vy.y * y + mat.vy.z * z,
             mat.vz.x * x + mat.vz.y * y + mat.vz.z * z };
}

template <typename T>
Vector3<T> Mulv(const Transform &t, const Vector3<T> &v, Vector3<T> &e)
{
    T x = v.x, y = v.y, z = v.z;
    m44f mat = t.mat();

    e.x = gamma(3)
          * (std::abs(mat.vx.x * x) + std::abs(mat.vx.y * y) + std::abs(mat.vx.z * z));
    e.y = gamma(3)
          * (std::abs(mat.vy.x * x) + std::abs(mat.vy.y * y) + std::abs(mat.vy.z * z));
    e.z = gamma(3)
          * (std::abs(mat.vz.x * x) + std::abs(mat.vz.y * y) + std::abs(mat.vz.z * z));

    return { mat.vx.x * x + mat.vx.y * y + mat.vx.z * z,
             mat.vy.x * x + mat.vy.y * y + mat.vy.z * z,
             mat.vz.x * x + mat.vz.y * y + mat.vz.z * z };
}

template <typename T>
Vector3<T> Mulv(const Transform &t,
                const Vector3<T> &v,
                const Vector3<T> &vError,
                Vector3<T> &tError)
{
    T x = v.x, y = v.y, z = v.z;
    m44f mat = t.mat();

    tError.x = gamma(3)
                   * (std::abs(mat.vx.x * x) + std::abs(mat.vx.y * y)
                      + std::abs(mat.vx.z * z))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(mat.vx.x * vError.x) + std::abs(mat.vx.y) * vError.x
                        + std::abs(mat.vx.z * vError.z));
    tError.y = gamma(3)
                   * (std::abs(mat.vy.x * x) + std::abs(mat.vy.y * y)
                      + std::abs(mat.vy.z * z))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(mat.vy.x * vError.x) + std::abs(mat.vy.y) * vError.x
                        + std::abs(mat.vy.z * vError.z));
    tError.z = gamma(3)
                   * (std::abs(mat.vz.x * x) + std::abs(mat.vx.z * y)
                      + std::abs(mat.vz.z * z))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(mat.vz.x * vError.x) + std::abs(mat.vz.y) * vError.x
                        + std::abs(mat.vz.z * vError.z));

    return { mat.vx.x * x + mat.vx.y * y + mat.vx.z * z,
             mat.vy.x * x + mat.vy.y * y + mat.vy.z * z,
             mat.vz.x * x + mat.vz.y * y + mat.vz.z * z };
}

template <typename T>
Vector3<T> Mulp(const Transform &t, const Vector3<T> &p)
{
    m44f mat = t.mat();
    T x = mat.vx.x * p.x + mat.vx.y * p.y + mat.vx.z * p.z + mat.vx.w;
    T y = mat.vy.x * p.x + mat.vy.y * p.y + mat.vy.z * p.z + mat.vy.w;
    T z = mat.vz.x * p.x + mat.vz.y * p.y + mat.vz.z * p.z + mat.vz.w;
    T w = mat.vw.x * p.x + mat.vw.y * p.y + mat.vw.z * p.z + mat.vw.w;

    if (FloatEqual(w, T{ 1.0 })) {
        return { x, y, z };
    }
    return { x / w, y / w, z / w };
}

template <typename T>
Vector3<T> Mulp(const Transform &t, const Vector3<T> &p, Vector3<T> &e)
{
    m44f mat = t.mat();
    T x = mat.vx.x * p.x + mat.vx.y * p.y + mat.vx.z * p.z + mat.vx.w;
    T y = mat.vy.x * p.x + mat.vy.y * p.y + mat.vy.z * p.z + mat.vy.w;
    T z = mat.vz.x * p.x + mat.vz.y * p.y + mat.vz.z * p.z + mat.vz.w;
    T w = mat.vw.x * p.x + mat.vw.y * p.y + mat.vw.z * p.z + mat.vw.w;

    e.x = gamma(3)
          * (std::abs(mat.vx.x * x) + std::abs(mat.vx.y * y) + std::abs(mat.vx.z * z)
             + std::abs(mat.vx.w));
    e.y = gamma(3)
          * (std::abs(mat.vy.x * x) + std::abs(mat.vy.y * y) + std::abs(mat.vy.z * z)
             + std::abs(mat.vy.w));
    e.z = gamma(3)
          * (std::abs(mat.vz.x * x) + std::abs(mat.vz.y * y) + std::abs(mat.vz.z * z)
             + std::abs(mat.vz.w));

    if (FloatEqual(w, T{ 1.0 })) {
        return { x, y, z };
    }
    return { x / w, y / w, z / w };
}

template <typename T>
Vector3<T> Mulp(const Transform &t,
                const Vector3<T> &p,
                const Vector3<T> &pError,
                Vector3<T> &tError)
{
    m44f mat = t.mat();
    T x = mat.vx.x * p.x + mat.vx.y * p.y + mat.vx.z * p.z + mat.vx.w;
    T y = mat.vy.x * p.x + mat.vy.y * p.y + mat.vy.z * p.z + mat.vy.w;
    T z = mat.vz.x * p.x + mat.vz.y * p.y + mat.vz.z * p.z + mat.vz.w;
    T w = mat.vw.x * p.x + mat.vw.y * p.y + mat.vw.z * p.z + mat.vw.w;

    tError.x = gamma(3)
                   * (std::abs(mat.vx.x * x) + std::abs(mat.vx.y * y)
                      + std::abs(mat.vx.z * z) + std::abs(mat.vx.w))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(mat.vx.x * pError.x) + std::abs(mat.vx.y) * pError.x
                        + std::abs(mat.vx.z * pError.z) + std::abs(mat.vx.w));
    tError.y = gamma(3)
                   * (std::abs(mat.vy.x * x) + std::abs(mat.vy.y * y)
                      + std::abs(mat.vy.z * z) + std::abs(mat.vy.w))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(mat.vy.x * pError.x) + std::abs(mat.vy.y) * pError.x
                        + std::abs(mat.vy.z * pError.z) + std::abs(mat.vy.w));
    tError.z = gamma(3)
                   * (std::abs(mat.vz.x * x) + std::abs(mat.vx.z * y)
                      + std::abs(mat.vz.z * z) + std::abs(mat.vz.w))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(mat.vz.x * pError.x) + std::abs(mat.vz.y) * pError.x
                        + std::abs(mat.vz.z * pError.z) + std::abs(mat.vz.w));

    if (FloatEqual(w, T{ 1.0 })) {
        return { x, y, z };
    }
    return { x / w, y / w, z / w };
}

/* Explicit template instantiations */
template Vector3<float> Mulv(const Transform &t, const Vector3<float> &v);
template Vector3<double> Mulv(const Transform &t, const Vector3<double> &v);
template Vector3<float> Mulv(const Transform &t,
                             const Vector3<float> &v,
                             Vector3<float> &e);
template Vector3<double> Mulv(const Transform &t,
                              const Vector3<double> &v,
                              Vector3<double> &e);
template Vector3<float> Mulv(const Transform &t,
                             const Vector3<float> &v,
                             const Vector3<float> &vError,
                             Vector3<float> &tError);
template Vector3<double> Mulv(const Transform &t,
                              const Vector3<double> &v,
                              const Vector3<double> &vError,
                              Vector3<double> &tError);

template Vector3<float> Mulp(const Transform &t, const Vector3<float> &p);
template Vector3<double> Mulp(const Transform &t, const Vector3<double> &p);
template Vector3<float> Mulp(const Transform &t,
                             const Vector3<float> &p,
                             Vector3<float> &e);
template Vector3<double> Mulp(const Transform &t,
                              const Vector3<double> &p,
                              Vector3<double> &e);
template Vector3<float> Mulp(const Transform &t,
                             const Vector3<float> &p,
                             const Vector3<float> &pError,
                             Vector3<float> &tError);
template Vector3<double> Mulp(const Transform &t,
                              const Vector3<double> &p,
                              const Vector3<double> &pError,
                              Vector3<double> &tError);

#pragma mark - Static Functions

Transform Transform::Scale(float x, float y, float z)
{
    float m[4][4] = { { x, 0.0f, 0.0f, 0.0f },
                      { 0.0f, y, 0.0f, 0.0f },
                      { 0.0f, 0.0f, z, 0.0f },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    float i[4][4] = { { 1.0f / x, 0.0f, 0.0f, 0.0f },
                      { 0.0f, 1.0f / y, 0.0f, 0.0f },
                      { 0.0f, 0.0f, 1.0f / z, 0.0f },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    return Transform(m44f(m), m44f(i));
}

Transform Transform::Translate(const v3f &v)
{
    float m[4][4] = { { 1.0f, 0.0f, 0.0f, v.x },
                      { 0.0f, 1.0f, 0.0f, v.y },
                      { 0.0f, 0.0f, 1.0f, v.z },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    float i[4][4] = { { 1.0f, 0.0f, 0.0f, -v.x },
                      { 0.0f, 1.0f, 0.0f, -v.y },
                      { 0.0f, 0.0f, 1.0f, -v.z },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    return Transform(m44f(m), m44f(i));
}

Transform Transform::RotateX(float theta)
{
    float cos = std::cos(Radians(theta));
    float sin = std::sin(Radians(theta));

    float m[4][4] = { { 1.0f, 0.0f, 0.0f, 0.0f },
                      { 0.0f, cos, -sin, 0.0f },
                      { 0.0f, sin, cos, 0.0f },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    return Transform(m44f(m), Transpose(m44f(m)));
}

Transform Transform::RotateY(float theta)
{
    float cos = std::cos(Radians(theta));
    float sin = std::sin(Radians(theta));

    float m[4][4] = { { cos, 0.0f, sin, 0.0f },
                      { 0.0f, 1.0f, 0.0f, 0.0f },
                      { -sin, 0.0f, cos, 0.0f },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    return Transform(m44f(m), Transpose(m44f(m)));
}

Transform Transform::RotateZ(float theta)
{
    float cos = std::cos(Radians(theta));
    float sin = std::sin(Radians(theta));

    float m[4][4] = { { cos, -sin, 0.0f, 0.0f },
                      { sin, cos, 0.0f, 0.0f },
                      { 0.0f, 0.0f, 1.0f, 0.0f },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };
    return Transform(m44f(m), Transpose(m44f(m)));
}

Transform Transform::LookAt(const v3f &p, const v3f &look, const v3f &up)
{
    v3f w = Normalize(p - look);
    v3f u = Normalize(Cross(up, w));
    v3f v = Cross(w, u);

    float m[4][4] = { { u.x, v.x, w.x, p.x },
                      { u.y, v.y, w.y, p.y },
                      { u.z, v.z, w.z, p.z },
                      { 0.0, 0.0, 0.0, 1.0 } };
    return Transform(m44f(m), Inverse(m44f(m)));
}

Transform Transform::Orthographic(float znear, float zfar)
{
    return Scale(1.0f, 1.0f, 1.0f / (zfar - znear)) * Translate({ 0.0f, 0.0f, -znear });
}

Transform Transform::Perspective(float fov, float znear, float zfar)
{
    ASSERT(fov > 0.0f);

    float itan = 1.0f / std::tan(Radians(fov / 2.0f));
    float A = zfar / (zfar - znear);
    float B = -zfar * znear / (zfar - znear);
    float m[4][4] = { { itan, 0.0f, 0.0f, 0.0f },
                      { 0.0f, itan, 0.0f, 0.0f },
                      { 0.0f, 0.0f, A, B },
                      { 0.0f, 0.0f, -1.0f, 0.0f } };
    return Transform(m);
}
