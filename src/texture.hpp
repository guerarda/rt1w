#ifndef TEXTURE_H
#define TEXTURE_H

#include "types.h"
#include "sptr.hpp"

struct Texture : Object {
    static sptr<Texture> create_color(const v3f &);
    static sptr<Texture> create_checker(const sptr<Texture> &, const sptr<Texture> &);
    static sptr<Texture> create_image(buffer_t *buf, const rect &r);

    virtual v3f value(float u, float v, const v3f &p) const = 0;
};

#endif
