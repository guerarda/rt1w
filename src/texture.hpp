#ifndef TEXTURE_H
#define TEXTURE_H

#include "types.h"
#include "sptr.hpp"

struct Texture : Object {
    static sptr<Texture> create_const(const v3f &);
    static sptr<Texture> create_checker(const sptr<Texture> &, const sptr<Texture> &);

    virtual v3f value(float u, float v, const v3f &p) const = 0;
};

#endif
