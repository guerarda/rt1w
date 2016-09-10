#include "sphere.hpp"
#include <math.h>

bool sphere::hit(const ray &r, float min, float max, hit_record &rec) const
{
    v3f oc = v3f_sub(r.m_org, m_center);
    float a = v3f_dot(r.m_dir, r.m_dir);
    float b = 2 * v3f_dot(r.m_dir, oc);
    float c = v3f_dot(oc, oc) - m_radius * m_radius;
    float delta = b * b - 4 * a * c;

    if (delta > 0.0f) {
        float t = (-b - sqrtf(delta)) / (2.0 * a);
        if (!(t > min && t < max)) {
            t = (-b + sqrtf(delta)) / (2.0 * a);
        }
        if (t > min && t < max) {
            rec.t = t;
            rec.p = r.point_at_param(t);
            rec.normal = v3f_sub(rec.p, m_center);

            return true;
        }
    }
    return false;
}
