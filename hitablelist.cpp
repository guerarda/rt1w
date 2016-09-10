#include "hitablelist.hpp"
#include <assert.h>
#include <vector>

struct _hitable_list : hitable_list {

    _hitable_list(size_t count, sptr<hitable> *l);
    virtual ~_hitable_list() { }

    bool hit(const sptr<ray> &r, float min, float max, hit_record &rec) const;

    std::vector<sptr<hitable>> m_hitables;
};

_hitable_list::_hitable_list(size_t count, sptr<hitable> *list)
{
    assert(list);
    m_hitables.assign(list, list + count);
}

bool _hitable_list::hit(const sptr<ray> &r, float min, float max, hit_record &rec) const
{
    hit_record tmp;
    bool hit = false;
    float t = max;

    for (sptr<hitable> h : m_hitables) {
        if (h->hit(r, min, t, tmp)) {
            hit = true;
            t = tmp.t;
            rec = tmp;
        }
    }
    return hit;
}

#pragma mark - Static constructors

sptr<hitable_list> hitable_list::create(size_t count, sptr<hitable> *l)
{
    return std::make_shared<_hitable_list>(count, l);
}
