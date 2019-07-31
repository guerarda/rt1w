#include "transform.hpp"

#include "error.h"
#include "ray.hpp"
#include "utils.hpp"

#include <cmath>

Transform Transform::operator*(const Transform &t) const
{
    return Transform(Mul(m_mat, t.m_mat), Mul(t.m_inv, m_inv));
}

Ray Transform::operator()(const Ray &r) const
{
    v3f o = Mulp(*this, r.org());
    v3f d = Mulv(*this, r.dir());

    return { o, d, r.max() };
}

Ray Transform::operator()(const Ray &r, v3f &oError, v3f &dError) const
{
    v3f o = Mulp(*this, r.org(), oError);
    v3f d = Mulv(*this, r.dir(), dError);

    return { o, d, r.max() };
}

Ray Transform::operator()(const Ray &r,
                          const v3f &oErrorIn,
                          const v3f &dErrorIn,
                          v3f &oErrorOut,
                          v3f &dErrorOut) const
{
    v3f o = Mulp(*this, r.org(), oErrorIn, oErrorOut);
    v3f d = Mulv(*this, r.dir(), dErrorIn, dErrorOut);

    return { o, d, r.max() };
}

bounds3f Transform::operator()(const bounds3f &b) const
{
    bounds3f rb;
    rb = Union(rb, Mulp(*this, v3f{ b.lo.x, b.lo.y, b.lo.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.lo.x, b.hi.y, b.lo.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.hi.x, b.hi.y, b.lo.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.hi.x, b.lo.y, b.lo.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.lo.x, b.lo.y, b.hi.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.lo.x, b.hi.y, b.hi.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.hi.x, b.hi.y, b.hi.z }));
    rb = Union(rb, Mulp(*this, v3f{ b.hi.x, b.lo.y, b.hi.z }));

    return rb;
}

Interaction Transform::operator()(const Interaction &i) const
{
    Interaction ri;
    ri.p = Mulp(*this, i.p, i.error, ri.error);
    ri.t = i.t;
    ri.uv = i.uv;
    ri.wo = Normalize(Mulv(*this, i.wo));
    ri.n = Normalize(Muln(*this, i.n));
    ri.dpdu = Mulv(*this, i.dpdu);
    ri.dpdv = Mulv(*this, i.dpdv);
    ri.shading.n = Normalize(Muln(*this, i.shading.n));
    ri.shading.dpdu = Mulv(*this, i.shading.dpdu);
    ri.shading.dpdv = Mulv(*this, i.shading.dpdv);
    ri.mat = i.mat;
    ri.prim = i.prim;

    return ri;
}

Transform Inverse(const Transform &t)
{
    return Transform(t.m_inv, t.m_mat);
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
    m44f m = t.m_mat;

    e.x = gamma(3) * (std::abs(m.vx.x * x) + std::abs(m.vx.y * y) + std::abs(m.vx.z * z));
    e.y = gamma(3) * (std::abs(m.vy.x * x) + std::abs(m.vy.y * y) + std::abs(m.vy.z * z));
    e.z = gamma(3) * (std::abs(m.vz.x * x) + std::abs(m.vz.y * y) + std::abs(m.vz.z * z));

    return { m.vx.x * x + m.vx.y * y + m.vx.z * z,
             m.vy.x * x + m.vy.y * y + m.vy.z * z,
             m.vz.x * x + m.vz.y * y + m.vz.z * z };
}

template <typename T>
Vector3<T> Mulv(const Transform &t,
                const Vector3<T> &v,
                const Vector3<T> &vError,
                Vector3<T> &tError)
{
    T x = v.x, y = v.y, z = v.z;
    m44f m = t.m_mat;

    tError.x = gamma(3)
                   * (std::abs(m.vx.x * x) + std::abs(m.vx.y * y) + std::abs(m.vx.z * z))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(m.vx.x * vError.x) + std::abs(m.vx.y * vError.y)
                        + std::abs(m.vx.z * vError.z));
    tError.y = gamma(3)
                   * (std::abs(m.vy.x * x) + std::abs(m.vy.y * y) + std::abs(m.vy.z * z))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(m.vy.x * vError.x) + std::abs(m.vy.y * vError.y)
                        + std::abs(m.vy.z * vError.z));
    tError.z = gamma(3)
                   * (std::abs(m.vz.x * x) + std::abs(m.vx.z * y) + std::abs(m.vz.z * z))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(m.vz.x * vError.x) + std::abs(m.vz.y * vError.y)
                        + std::abs(m.vz.z * vError.z));

    return { m.vx.x * x + m.vx.y * y + m.vx.z * z,
             m.vy.x * x + m.vy.y * y + m.vy.z * z,
             m.vz.x * x + m.vz.y * y + m.vz.z * z };
}

template <typename T>
Vector3<T> Mulp(const Transform &t, const Vector3<T> &p)
{
    m44f m = t.m_mat;
    T x = m.vx.x * p.x + m.vx.y * p.y + m.vx.z * p.z + m.vx.w;
    T y = m.vy.x * p.x + m.vy.y * p.y + m.vy.z * p.z + m.vy.w;
    T z = m.vz.x * p.x + m.vz.y * p.y + m.vz.z * p.z + m.vz.w;
    T w = m.vw.x * p.x + m.vw.y * p.y + m.vw.z * p.z + m.vw.w;

    if (FloatEqual(w, T{ 1.0 })) {
        return { x, y, z };
    }
    return { x / w, y / w, z / w };
}

template <typename T>
Vector3<T> Mulp(const Transform &t, const Vector3<T> &p, Vector3<T> &e)
{
    m44f m = t.m_mat;
    T x = m.vx.x * p.x + m.vx.y * p.y + m.vx.z * p.z + m.vx.w;
    T y = m.vy.x * p.x + m.vy.y * p.y + m.vy.z * p.z + m.vy.w;
    T z = m.vz.x * p.x + m.vz.y * p.y + m.vz.z * p.z + m.vz.w;
    T w = m.vw.x * p.x + m.vw.y * p.y + m.vw.z * p.z + m.vw.w;

    e.x = gamma(3)
          * (std::abs(m.vx.x * x) + std::abs(m.vx.y * y) + std::abs(m.vx.z * z)
             + std::abs(m.vx.w));
    e.y = gamma(3)
          * (std::abs(m.vy.x * x) + std::abs(m.vy.y * y) + std::abs(m.vy.z * z)
             + std::abs(m.vy.w));
    e.z = gamma(3)
          * (std::abs(m.vz.x * x) + std::abs(m.vz.y * y) + std::abs(m.vz.z * z)
             + std::abs(m.vz.w));

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
    m44f m = t.m_mat;
    T x = m.vx.x * p.x + m.vx.y * p.y + m.vx.z * p.z + m.vx.w;
    T y = m.vy.x * p.x + m.vy.y * p.y + m.vy.z * p.z + m.vy.w;
    T z = m.vz.x * p.x + m.vz.y * p.y + m.vz.z * p.z + m.vz.w;
    T w = m.vw.x * p.x + m.vw.y * p.y + m.vw.z * p.z + m.vw.w;

    tError.x = gamma(3)
                   * (std::abs(m.vx.x * x) + std::abs(m.vx.y * y) + std::abs(m.vx.z * z)
                      + std::abs(m.vx.w))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(m.vx.x * pError.x) + std::abs(m.vx.y * pError.y)
                        + std::abs(m.vx.z * pError.z));
    tError.y = gamma(3)
                   * (std::abs(m.vy.x * x) + std::abs(m.vy.y * y) + std::abs(m.vy.z * z)
                      + std::abs(m.vy.w))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(m.vy.x * pError.x) + std::abs(m.vy.y * pError.y)
                        + std::abs(m.vy.z * pError.z));
    tError.z = gamma(3)
                   * (std::abs(m.vz.x * x) + std::abs(m.vx.z * y) + std::abs(m.vz.z * z)
                      + std::abs(m.vz.w))
               + (T{ 1.0 } + gamma(3))
                     * (std::abs(m.vz.x * pError.x) + std::abs(m.vz.y * pError.y)
                        + std::abs(m.vz.z * pError.z));

    if (FloatEqual(w, T{ 1.0 })) {
        return { x, y, z };
    }
    return { x / w, y / w, z / w };
}

template <typename T>
Vector3<T> Muln(const Transform &t, const Vector3<T> &n)
{
    T x = t.m_inv.vx.x * n.x + t.m_inv.vy.x * n.y + t.m_inv.vz.x * n.z;
    T y = t.m_inv.vx.y * n.x + t.m_inv.vy.y * n.y + t.m_inv.vz.y * n.z;
    T z = t.m_inv.vx.z * n.x + t.m_inv.vy.z * n.y + t.m_inv.vz.z * n.z;

    return { x, y, z };
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

template Vector3<float> Muln(const Transform &t, const Vector3<float> &n);
template Vector3<double> Muln(const Transform &t, const Vector3<double> &n);

#pragma mark - Static Functions

Transform Transform::Scale(float s)
{
    return Transform::Scale(s, s, s);
}

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

Transform Transform::Rotate(float theta, const v3f &axis)
{
    v3f a = Normalize(axis);
    float sinTheta = std::sin(Radians(theta));
    float cosTheta = std::cos(Radians(theta));
    m44f m = m44f_identity();

    m.vx.x = a.x * a.x + (1 - a.x * a.x) * cosTheta;
    m.vx.y = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
    m.vx.z = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
    m.vx.w = 0;

    m.vy.x = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
    m.vy.y = a.y * a.y + (1 - a.y * a.y) * cosTheta;
    m.vy.z = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
    m.vy.w = 0;

    m.vz.x = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
    m.vz.y = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
    m.vz.z = a.z * a.z + (1 - a.z * a.z) * cosTheta;
    m.vz.w = 0;

    return Transform(m, Transpose(m));
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
