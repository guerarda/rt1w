#ifndef HITABLELIST_H
#define HITABLELIST_H

#include "hitable.hpp"
#include <stddef.h>

struct hitable_list : hitable {
    hitable_list(size_t count, hitable **l);
    virtual ~hitable_list() { }

    virtual bool hit(const ray &r, float min, float max, hit_record &rec) const;

private:
    hitable **m_list;
    size_t    m_count;
};

#endif
