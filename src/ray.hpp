#pragma once

#include "geometry.hpp"
#include "sptr.hpp"

struct Ray : Object {
    static sptr<Ray> create(const v3f &org, const v3f &dir);

    virtual v3f origin() const = 0;
    virtual v3f direction() const = 0;

    virtual v3f point(float t) const = 0;
};
