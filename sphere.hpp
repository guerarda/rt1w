#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.hpp"

struct sphere : hitable {
    sphere() { }
    sphere(v3f c, float r) : m_center(c), m_radius(r) { }

    virtual ~sphere() { }

    bool hit(const ray &r, float min, float max, hit_record &rec) const;

    v3f   m_center;
    float m_radius;
};

#endif
