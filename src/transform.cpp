#include "transform.hpp"

#include <math.h>
#include "error.h"

static inline float Radians(float deg)
{
    return (float)((M_PI / 180.0) * (double)deg);
}

Transform Transform::operator * (const Transform &t) const
{
    return Transform(Mul(m_mat, t.mat()), Mul(t.inv(), m_inv));
}

sptr<ray> Transform::operator () (const sptr<ray> &r) const
{
    v3f o = Mulp(m_mat, r->origin());
    v3f d = Mulv(m_mat, r->direction());

    return ray::create(o, d);
}

Transform Inverse(const Transform &t)
{
    return Transform(t.inv(), t.mat());
}

Transform Transform::Scale(float x, float y, float z)
{
    float m[4][4]  = {
        { x,    0.0f, 0.0f, 0.0f },
        { 0.0f, y,    0.0f, 0.0f },
        { 0.0f, 0.0f, z,    0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    float i[4][4] = {
        { 1.0f / x, 0.0f,     0.0f,     0.0f },
        { 0.0f,     1.0f / y, 0.0f,     0.0f },
        { 0.0f,     0.0f,     1.0f / z, 0.0f },
        { 0.0f,     0.0f,     0.0f,     1.0f }
    };
    return Transform(m44f(m), m44f(i));
}

Transform Transform::Translate(const v3f &v)
{
    float m[4][4] = {
        { 1.0f, 0.0f ,0.0f, v.x  },
        { 0.0f, 1.0f, 0.0f, v.y  },
        { 0.0f, 0.0f, 1.0f, v.z  },
        { 0.0f ,0.0f, 0.0f, 1.0f }
    };
    float i[4][4] = {
        { 1.0f, 0.0f ,0.0f, -v.x },
        { 0.0f, 1.0f, 0.0f, -v.y },
        { 0.0f, 0.0f, 1.0f, -v.z },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Transform(m44f(m), m44f(i));
}

Transform Transform::RotateX(float theta)
{
    float cos = std::cos(Radians(theta));
    float sin = std::sin(Radians(theta));

    float m[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, cos, -sin,  0.0f },
        { 0.0f, sin,  cos,  0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Transform(m44f(m), Transpose(m44f(m)));
}

Transform Transform::RotateY(float theta)
{
    float cos = std::cos(Radians(theta));
    float sin = std::sin(Radians(theta));

    float m[4][4] = {
        { cos,  0.0f, sin,  0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { -sin, 0.0f, cos,  0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Transform(m44f(m), Transpose(m44f(m)));
}

Transform Transform::RotateZ(float theta)
{
    float cos = std::cos(Radians(theta));
    float sin = std::sin(Radians(theta));

    float m[4][4] = {
        { cos, -sin,  0.0f, 0.0f },
        { sin,  cos,  0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Transform(m44f(m), Transpose(m44f(m)));
}

Transform Transform::LookAt(const v3f &p, const v3f &look, const v3f &up)
{
    v3f w = Normalize(p - look);
    v3f u = Normalize(Cross(up, w));
    v3f v = Cross(w, u);

    float m[4][4] = {
        { u.x,  v.x,  w.x,  p.x  },
        { u.y,  v.y,  w.y,  p.y  },
        { u.z,  v.z,  w.z,  p.z  },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Transform(m44f(m), Inverse(m44f(m)));
}

Transform Transform::Orthographic(float near, float far)
{
    return Scale(1.0f, 1.0f, 1.0f / (far - near)) * Translate({ 0.0f, 0.0f, -near });
}

Transform Transform::Perspective(float fov, float n, float f)
{
    ASSERT(fov > 0.0f);

    float itan = 1.0f / std::tan(Radians(fov / 2.0f));
    float A = f / (f - n);
    float B = - f * n / (f - n);
    float m[4][4] = {
        { itan, 0.0f, 0.0f, 0.0f  },
        { 0.0f, itan, 0.0f, 0.0f  },
        { 0.0f, 0.0f,    A,    B  },
        { 0.0f, 0.0f, -1.0f, 0.0f }
    };
    return Transform(m);
}