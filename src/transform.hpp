#pragma once

#include "geometry.hpp"
#include "ray.hpp"
#include "utils.hpp"

struct Transform {
    Transform() : m_mat(m44f_identity()), m_inv(m44f_identity()) {}
    explicit Transform(const m44f &m) : m_mat(m), m_inv(Inverse(m)) {}
    explicit Transform(const m44f &m, const m44f &inv) : m_mat(m), m_inv(inv) {}

    m44f mat() const { return m_mat; }
    m44f inv() const { return m_inv; }

    Transform operator*(const Transform &)const;
    sptr<Ray> operator()(const sptr<Ray> &r) const;
    bounds3f operator()(const bounds3f &) const;

    static Transform Scale(float x, float y, float z);
    static Transform Translate(const v3f &v);
    static Transform RotateX(float theta);
    static Transform RotateY(float theta);
    static Transform RotateZ(float theta);
    static Transform LookAt(const v3f &p, const v3f &look, const v3f &up);
    static Transform Orthographic(float znear, float zfar);
    static Transform Perspective(float fov, float znear, float zfar);

private:
    m44f m_mat;
    m44f m_inv;
};

Transform Inverse(const Transform &);

#pragma mark - Inline Functions

template <typename T>
inline Vector3<T> Mulv(const Transform &t, const Vector3<T> &v)
{
    T x = v.x, y = v.y, z = v.z;
    m44f mat = t.mat();
    return { mat.vx.x * x + mat.vx.y * y + mat.vx.z * z,
             mat.vy.x * x + mat.vy.y * y + mat.vy.z * z,
             mat.vz.x * x + mat.vz.y * y + mat.vz.z * z };
}

template <typename T>
inline Vector3<T> Mulp(const Transform &t, const Vector3<T> &p)
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
