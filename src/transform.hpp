#pragma once

#include "geometry.hpp"

struct Ray;

struct Transform {
    Transform() : m_mat(m44f_identity()), m_inv(m44f_identity()) {}
    explicit Transform(const m44f &m) : m_mat(m), m_inv(Inverse(m)) {}
    explicit Transform(const m44f &m, const m44f &inv) : m_mat(m), m_inv(inv) {}

    m44f mat() const { return m_mat; }
    m44f inv() const { return m_inv; }

    Transform operator*(const Transform &)const;
    Ray operator()(const Ray &r) const;
    Ray operator()(const Ray &r, v3f &oError, v3f &dError) const;
    Ray operator()(const Ray &r,
                   const v3f &oErrorIn,
                   const v3f &dErrorIn,
                   v3f &oErrorOut,
                   v3f &dErrorOut) const;
    bounds3f operator()(const bounds3f &) const;

    static Transform Scale(float x, float y, float z);
    static Transform Translate(const v3f &v);
    static Transform RotateX(float theta);
    static Transform RotateY(float theta);
    static Transform RotateZ(float theta);
    static Transform LookAt(const v3f &p, const v3f &look, const v3f &up);
    static Transform Orthographic(float znear, float zfar);
    static Transform Perspective(float fov, float znear, float zfar);

    friend Transform Inverse(const Transform &);

    /* Transform a Vector */
    template <typename T>
    friend Vector3<T> Mulv(const Transform &t, const Vector3<T> &v);
    template <typename T>
    friend Vector3<T> Mulv(const Transform &t, const Vector3<T> &v, Vector3<T> &error);
    template <typename T>
    friend Vector3<T> Mulv(const Transform &t,
                           const Vector3<T> &v,
                           const Vector3<T> &vError,
                           Vector3<T> &tError);

    /* Transform a Point */
    template <typename T>
    friend Vector3<T> Mulp(const Transform &t, const Vector3<T> &p);
    template <typename T>
    friend Vector3<T> Mulp(const Transform &t, const Vector3<T> &p, Vector3<T> &error);
    template <typename T>
    friend Vector3<T> Mulp(const Transform &t,
                           const Vector3<T> &p,
                           const Vector3<T> &pError,
                           Vector3<T> &tError);

    /* Transform a Normal */
    template <typename T>
    friend Vector3<T> Muln(const Transform &t, const Vector3<T> &n);

private:
    m44f m_mat;
    m44f m_inv;
};
