#include "integrator.hpp"

#include <limits>

#include "light.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "ray.hpp"
#include "sampler.hpp"
#include "scene.hpp"

#pragma mark - Integrator Implementation

struct _Integrator : Integrator {
    _Integrator(const sptr<Sampler> &s, size_t m) : m_sampler(s), m_maxDepth(m) { }

    sptr<const Sampler> sampler() const override { return m_sampler; }
    v3f Li(const sptr<Ray> &ray,
           const sptr<Scene> &scene,
           const sptr<Sampler> &sampler,
           size_t depth) const override;

    sptr<Sampler> m_sampler;
    size_t        m_maxDepth;
    v3f           m_background;
};

v3f _Integrator::Li(const sptr<Ray> &ray,
                    const sptr<Scene> &scene,
                    const sptr<Sampler> &sampler,
                    size_t depth) const
{
    hit_record rec;
    if (scene->world()->hit(ray, 0.001f, std::numeric_limits<float>::max(), rec)) {
        v3f attenuation;
        v3f wi;
        if (depth < m_maxDepth && rec.mat->scatter(ray, rec, attenuation, wi)) {
            v3f L;
            for (const auto &light : scene->lights()) {
                v3f lwi;
                v3f Li = light->sample_Li(rec, lwi);
                if (light->visible(rec, scene)) {
                    L += Li * rec.mat->f(rec, rec.wo, lwi);
                }
            }
            sptr<Ray> scattered = Ray::create(rec.p, wi);
            L +=  attenuation * Li(scattered, scene, sampler, depth + 1);
            return L;
        }
    }
    return m_background;
}

#pragma mark - Static constructor

sptr<Integrator> Integrator::create(const sptr<Sampler> &sampler, size_t maxDepth)
{
    return std::make_shared<_Integrator>(sampler, maxDepth);
}
