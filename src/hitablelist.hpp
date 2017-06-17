#ifndef HITABLELIST_H
#define HITABLELIST_H

#include "hitable.hpp"
#include "sptr.hpp"

struct Hitable_list : Hitable {

    static sptr<Hitable_list> create(size_t count, sptr<Hitable> *l);

    virtual bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
    virtual bounds3f bounds() const = 0;
};

#endif
