#include "texture.hpp"
#include <math.h>

struct _Texture_const : Texture {
    _Texture_const(const v3f &c) : m_color(c) { };

    v3f value(float, float, const v3f &) const { return m_color; }

    v3f m_color;
};

struct _Texture_checker : Texture {

    _Texture_checker(const sptr<Texture> &a,
                     const sptr<Texture> &b) : m_odd(a), m_even(b) { }

    v3f value(float, float, const v3f &) const;

    sptr<Texture> m_odd;
    sptr<Texture> m_even;
};

v3f _Texture_checker::value(float u, float v, const v3f &p) const
{
    float sines = sinf(10.0f * p.x) * sinf(10.0f * p.y) * sinf(10.0f * p.z);
    return sines > 0.0f ? m_even->value(u, v, p) : m_odd->value(u, v, p);
}

#pragma mark - Static constructors

sptr<Texture> Texture::create_const(const v3f &c)
{
    return std::make_shared<_Texture_const>(c);
}

sptr<Texture> Texture::create_checker(const sptr<Texture> &a, const sptr<Texture> &b)
{
    return std::make_shared<_Texture_checker>(a, b);
}
