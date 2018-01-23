#include "texture.hpp"
#include <assert.h>
#include <math.h>
#include "value.hpp"
#include "params.hpp"
#include "error.h"

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

struct _Texture_img : Texture {

    _Texture_img(buffer_t *buf, const rect_t &r);
    ~_Texture_img();

    v3f value(float, float, const v3f &) const;

    buffer_t *m_buf;
    rect_t    m_rect;
};

_Texture_img::_Texture_img(buffer_t *buf, const rect_t &r)
{
    assert(buf);
    assert(buf->data);

    m_buf = buf;
    m_rect = r;
}

_Texture_img::~_Texture_img()
{
    if (m_buf->data) {
        free(m_buf->data);
    }
}

v3f _Texture_img::value(float u, float v, const v3f &) const
{
    uint32_t x, y;

    u = fminf(1.0f, fmaxf(0.0f, u));
    v = fminf(1.0f, fmaxf(0.0f, v));
    x = (uint32_t)lrint(u * (float)(m_rect.size.x - 1));
    y = (uint32_t)lrint((1.0f - v) * (float)(m_rect.size.y - 1));

    size_t pix_size = m_buf->format.size;
    uint8_t *sp = (uint8_t *)m_buf->data + y * m_buf->bpr + x * pix_size;
    v3f color = {
        sp[0] / 255.0f,
        sp[1] / 255.0f,
        sp[2] / 255.0f
    };

    return color;
}

#pragma mark - Static constructors

sptr<Texture> Texture::create_color(const v3f &c)
{
    return std::make_shared<_Texture_const>(c);
}

sptr<Texture> Texture::create_checker(const sptr<Texture> &a,
                                      const sptr<Texture> &b)
{
    return std::make_shared<_Texture_checker>(a, b);
}

sptr<Texture> Texture::create_image(buffer_t *b, const rect &r)
{
    return std::make_shared<_Texture_img>(b, r);
}

sptr<Texture> Texture::create(const sptr<Params> &p)
{
    std::string type = p->string("type");
    WARNING_IF(type.empty(), "Texture parameter \"type\" not specified");

    if (type == "color") {
        if (sptr<Value> v = p->value("color")) {
            return Texture::create_color(v->vector3f());
        }
        warning("Texture parameter \"color\" not specified");
    }
    else if (type == "checker") {
        sptr<Texture> even = p->texture("even");
        sptr<Texture> odd = p->texture("odd");

        if (even && odd) {
            return Texture::create_checker(even, odd);
        }
        WARNING_IF(!even, "Texture parameter \"even\" not specified");
        WARNING_IF(!odd, "Texture parameter \"odd\" not specified");
    }
    else if (type == "image") {

    }
    else {
        warning("Texture parameter \"type\" not recognized");
    }

    return nullptr;
}
