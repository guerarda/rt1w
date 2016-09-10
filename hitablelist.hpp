#ifndef HITABLELIST_H
#define HITABLELIST_H

#include "hitable.hpp"
#include "sptr.hpp"

struct hitable_list;

struct hitable_list : hitable {

    static sptr<hitable_list> create(size_t count, sptr<hitable> *l);

    virtual bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const = 0;
};

#endif
