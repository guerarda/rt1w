#pragma once

#include "geometry.hpp"

struct Ray {
    Ray(v3f org, v3f dir) : m_org(org), m_dir(dir) {}

    v3f org() const { return m_org; }
    v3f dir() const { return m_dir; }
    v3f operator()(float t) const { return m_org + t * m_dir; }

private:
    v3f m_org;
    v3f m_dir;
};
