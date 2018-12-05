#include "texture.hpp"

#include "error.h"
#include "image.hpp"
#include "params.hpp"
#include "value.hpp"

#include <cmath>

struct _Texture_const : Texture {
    _Texture_const(const v3f &c) : m_color(c){};

    v3f value(float, float, const v3f &) const override { return m_color; }

    v3f m_color;
};

struct _Texture_checker : Texture {
    _Texture_checker(const sptr<Texture> &a, const sptr<Texture> &b) : m_odd(a), m_even(b)
    {}

    v3f value(float, float, const v3f &) const override;

    sptr<Texture> m_odd;
    sptr<Texture> m_even;
};

v3f _Texture_checker::value(float u, float v, const v3f &p) const
{
    float sines = sinf(10.0f * p.x) * sinf(10.0f * p.y) * sinf(10.0f * p.z);
    return sines > 0.0f ? m_even->value(u, v, p) : m_odd->value(u, v, p);
}

struct _Texture_img : Texture {
    _Texture_img(const sptr<Image> &img, rect_t r);

    v3f value(float, float, const v3f &) const override;

    sptr<Image> m_img;
    rect_t m_rect;
};

_Texture_img::_Texture_img(const sptr<Image> &img, rect_t r)
{
    ASSERT(img);
    m_img = img;
    m_rect = r;
}

v3f _Texture_img::value(float u, float v, const v3f &) const
{
    buffer_t buf = m_img->buffer();

    u = fminf(1.0f, fmaxf(0.0f, u));
    v = fminf(1.0f, fmaxf(0.0f, v));

    /* Convert (u, v) to (x, y) in m_rect */
    int64_t x = lrint((double)u * (double)(m_rect.size.x - 1));
    int64_t y = lrint((1.0 - (double)v) * (double)(m_rect.size.y - 1));

    /* Convert coordinated from m_rect to buf.rect */
    x += m_rect.org.x;
    y += m_rect.org.y;

    /* Convert from buf.rect to offset in buffer.data */
    x += buf.rect.org.x;
    y += buf.rect.org.y;

    ASSERT(x >= 0 && y >= 0);

    size_t pix_size = buf.format.size;
    uint8_t *sp = (uint8_t *)buf.data + (size_t)y * buf.bpr + (size_t)x * pix_size;
    v3f color = { sp[0] / 255.0f, sp[1] / 255.0f, sp[2] / 255.0f };

    return color;
}

#pragma mark - Static constructors

sptr<Texture> Texture::create_color(const v3f &c)
{
    return std::make_shared<_Texture_const>(c);
}

sptr<Texture> Texture::create_checker(const sptr<Texture> &a, const sptr<Texture> &b)
{
    return std::make_shared<_Texture_checker>(a, b);
}

sptr<Texture> Texture::create_image(const sptr<Image> &img, rect_t r)
{
    return std::make_shared<_Texture_img>(img, r);
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
        if (sptr<Image> img = Image::create(p)) {
            v2i org = Params::vector2i(p, "origin", { 0, 0 });
            v2u size = Params::vector2u(p, "size", { 0, 0 });
            rect_t rect = { { org.x, org.y }, { size.x, size.y } };
            return Texture::create_image(img, rect);
        }
        warning("Texture parameter \"image\" not specified");

        return nullptr;
    }
    warning("Texture parameter \"type\" not recognized");

    return nullptr;
}
