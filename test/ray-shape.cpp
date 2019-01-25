#include "catch.hpp"

#include "geometry.hpp"
#include "interaction.hpp"
#include "ray.hpp"
#include "rng.hpp"
#include "sampling.hpp"
#include "sphere.hpp"
#include "utils.hpp"

static float pExp(RNG &rng, float minExp = -8.f, float maxExp = 8.f)
{
    float logu = Lerp(rng.f32(), minExp, maxExp);
    float sign = rng.f32() < .5 ? -1 : +1;
    return sign * std::powf(10, logu);
}

static v3f RandomPoint(RNG &rng)
{
    return { pExp(rng), pExp(rng), pExp(rng) };
}

static bool ReintersectShape(const sptr<Shape> &shape, const v3f org, RNG &rng)
{
    /* Random target in the shape bounding box */
    bounds3f box = shape->bounds();
    v3f dst = { Lerp(rng.f32(), box.lo.x, box.hi.x),
                Lerp(rng.f32(), box.lo.y, box.hi.y),
                Lerp(rng.f32(), box.lo.z, box.hi.z) };

    Ray ray = { org, dst - org };

    Interaction isect;
    if (!shape->intersect(ray, isect)) {
        return false;
    }

    /* Shoot rays in random directions from the intersection */
    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        v3f w = UniformSampleSphere({ rng.f32(), rng.f32() });

        /* Make sure w and n are in the same hemisphere */
        w = FaceForward(w, isect.n);

        Ray r = SpawnRay(isect, w);
        if (shape->intersect(r, isect)) {
            ++n;
        }
    }
    return n != 0;
}

TEST_CASE("Sphere", "[shape], [isect]")
{
    uptr<RNG> rng = RNG::create();

    /* Create random Sphere */
    v3f center = RandomPoint(*rng);
    float radius = pExp(*rng, .0f, 3.f);
    sptr<Sphere> sphere = Sphere::create(center, radius);

    /* Random Ray origin outside the sphere' bounding box */
    v3f org;
    do {
        org = RandomPoint(*rng);
    } while (!sphere->bounds().includes(org));

    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        if (ReintersectShape(sphere, org, *rng)) {
            ++n;
        }
    }
    REQUIRE(n == 0);
}
