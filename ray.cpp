#include "ray.hpp"

v3f ray::point_at_param(float t) const
{
    return v3f_add(m_org, v3f_smul(t, m_dir));
}
