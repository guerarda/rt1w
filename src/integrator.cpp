#include "integrator.hpp"

#include <limits>

#include "camera.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "ray.hpp"

#pragma mark - Integrator Implementation

struct _Integrator : Integrator {
    _Integrator(const sptr<Sampler> &sampler, size_t maxDepth) : m_sampler(sampler), m_maxDepth(maxDepth) { }

    virtual sptr<const Sampler> sampler() const override { return m_sampler; }
    virtual v3f Li(const sptr<Ray> &ray, const sptr<Primitive> &world, size_t depth) const override;

    sptr<Sampler> m_sampler;
    size_t        m_maxDepth;
    v3f           m_background;
};

v3f _Integrator::Li(const sptr<Ray> &ray, const sptr<Primitive> &world, size_t depth) const
{
    hit_record rec;
    if (world->hit(ray, 0.001f, std::numeric_limits<float>::max(), rec)) {
        v3f attenuation;
        v3f emitted = rec.mat->emitted(rec.uv.x, rec.uv.y, rec.p);
        v3f wi;

        if (depth < m_maxDepth && rec.mat->scatter(ray, rec, attenuation, wi)) {
            sptr<Ray> scattered = Ray::create(rec.p, wi);
            return emitted + attenuation * Li(scattered, world, depth + 1);
        }
        return emitted;
    }
    return m_background;
}

#pragma mark - Static constructor

sptr<Integrator> Integrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_Integrator>(sampler, maxDepth);
}
