#ifndef RAY_H
#define RAY_H

#include "vec.hpp"

struct ray {
    ray(const v3f &org, const v3f dir) : m_org(org), m_dir(dir) { }

    v3f m_org;
    v3f m_dir;
};

#endif
