#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include "catch.hpp"

#include "rt1w/accelerator.hpp"
#include "rt1w/geometry.hpp"
#include "rt1w/integrator.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/rng.hpp"
#include "rt1w/sampler.hpp"
#include "rt1w/scene.hpp"
#include "rt1w/utils.hpp"

#include <algorithm>
#include <vector>

static std::vector<Ray> CameraRays(const sptr<Camera> &camera)
{
    auto ns = 1u;
    auto sampler = Sampler::create(ns, ns, 4, true);

    /* Generate Rays */
    std::vector<Ray> rays;
    for (size_t x = 0; x < camera->resolution().x; ++x) {
        for (size_t y = 0; y < camera->resolution().y; ++y) {
            sampler->startPixel({ (int32_t)x, (int32_t)y });
            do {
                rays.push_back(camera->generateRay(sampler->cameraSample()));
            } while (sampler->startNextSample());
        }
    }
    return rays;
}

TEST_CASE("Accelerators", "[bvh][qbvh]")
{
    auto file = "../../scenes/cornell.json";
    auto render = RenderDescription::load(file);
    auto rays = CameraRays(render->camera());

    auto bvh = Accelerator::create("bvh", render->primitives());
    auto qbvh = Accelerator::create("qbvh", render->primitives());

    SECTION("QBVH Intersect Accuracy")
    {
        for (const auto &ray : rays) {
            Interaction ibvh, iqbvh;
            bool b = bvh->intersect(ray, ibvh);
            bool q = qbvh->intersect(ray, iqbvh);

            REQUIRE(b == q);
            if (b && q) {
                REQUIRE(ibvh.prim == iqbvh.prim);
                REQUIRE(FloatEqual(ibvh.t, iqbvh.t));
            }
        }
    }

    SECTION("QBVH QIntersect Accuracy")
    {
        for (const auto &ray : rays) {
            Interaction ibvh, iqbvh;
            bool b = bvh->qIntersect(ray);
            bool q = qbvh->qIntersect(ray);

            REQUIRE(b == q);
        }
    }

    SECTION("QBVH Intersect Performance")
    {
        BENCHMARK("BVH Intersect")
        {
            auto min = (float)Infinity;
            for (const auto &r : rays) {
                Interaction isect;
                bvh->intersect(r, isect);
                min = std::min(min, isect.t);
            }
            return min;
        };

        BENCHMARK("QBVH Intersect")
        {
            auto min = (float)Infinity;
            for (const auto &r : rays) {
                Interaction isect;
                bvh->intersect(r, isect);
                min = std::min(min, isect.t);
            }
            return min;
        };
    }

    SECTION("QBVH QIntersect Performance")
    {
        BENCHMARK("BVH QIntersect")
        {
            bool b = true;
            for (const auto &r : rays) {
                b &= bvh->qIntersect(r);
            }
            return b;
        };

        BENCHMARK("QBVH QIntersect")
        {
            bool b = true;
            for (const auto &r : rays) {
                Interaction isect;
                b &= bvh->qIntersect(r);
            }
            return b;
        };
    }
}
