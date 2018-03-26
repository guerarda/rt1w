#pragma once

#include "shape.hpp"
#include "sptr.hpp"
#include <vector>

struct Params;
struct Value;

struct Mesh : Shape {

    static sptr<Mesh> create(size_t nt,
                             const sptr<Value> &v,
                             const sptr<Value> &i,
                             const sptr<Value> &n,
                             const sptr<Value> &uv);

    static sptr<Mesh> create(const sptr<Params> &params);

    virtual bool     hit(const sptr<ray> &r,
                         float min,
                         float max,
                         hit_record &rec) const = 0;
    virtual bounds3f bounds() const = 0;
};
