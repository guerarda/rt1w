#pragma once

#include "geometry.hpp"
#include "utils.hpp"

struct Interaction;

struct Ray {
    Ray(v3f org, v3f dir) : m_org(org), m_dir(dir) {}
    Ray(v3f org, v3f dir, float max) : m_org(org), m_dir(dir), m_max(max) {}
    Ray(const Ray &r, float max) : m_org(r.org()), m_dir(r.dir()), m_max(max) {}

    v3f org() const { return m_org; }
    v3f dir() const { return m_dir; }
    float max() const { return m_max; }

    v3f operator()(float t) const { return m_org + t * m_dir; }

private:
    v3f m_org;
    v3f m_dir;
    float m_max = Infinity;
};

Ray SpawnRay(const Interaction &i, v3f dir);
Ray SpawnRayTo(const Interaction &i, v3f p);
Ray SpawnRayTo(const Interaction &i, const Interaction &t);
