#pragma once

#include "geometry.hpp"
#include "sptr.hpp"
#include "types.h"

struct Image;
struct Params;

struct Texture : Object {
    static sptr<Texture> create(const sptr<Params> &params);

    static sptr<Texture> create_color(const v3f &);
    static sptr<Texture> create_checker(const sptr<Texture> &a, const sptr<Texture> &b);
    static sptr<Texture> create_image(const sptr<Image> &img, rect_t r);

    virtual v3f value(float u, float v, const v3f &p) const = 0;
};
