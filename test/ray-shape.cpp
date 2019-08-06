#include "catch.hpp"

#include "rt1w/geometry.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/mesh.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/rng.hpp"
#include "rt1w/sampling.hpp"
#include "rt1w/sphere.hpp"
#include "rt1w/transform.hpp"
#include "rt1w/utils.hpp"

static float pExp(RNG &rng, float minExp = -8.f, float maxExp = 8.f)
{
    float logu = Lerp(rng.f32(), minExp, maxExp);
    float sign = rng.f32() < .5 ? -1 : +1;
    return sign * std::powf(10, logu);
}

static v3f RandomPoint(RNG &rng, float minExp = -8.f, float maxExp = 8)
{
    return { pExp(rng, minExp, maxExp),
             pExp(rng, minExp, maxExp),
             pExp(rng, minExp, maxExp) };
}

static sptr<Shape> RandomSphere(RNG &rng)
{
    Transform t = Transform::Translate(RandomPoint(rng));
    float radius = pExp(rng, .0f, 3.f);
    return Sphere::create(t, radius);
}

static sptr<Shape> RandomTriangle(RNG &rng)
{
    auto vertices = std::make_unique<std::vector<v3f>>(3);
    vertices->push_back(RandomPoint(rng, .0f, 3.f));
    vertices->push_back(RandomPoint(rng, .0f, 3.f));
    vertices->push_back(RandomPoint(rng, .0f, 3.f));

    auto normals = uptr<std::vector<v3f>>();
    auto texcoords = uptr<std::vector<v2f>>();

    sptr<VertexData> vd = VertexData::create(3, vertices, normals, texcoords);

    auto indices = std::make_shared<std::vector<uint32_t>>(3);
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);

    return Mesh::create(1, vd, indices, Transform());
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
        Interaction risect;
        if (shape->intersect(r, risect)) {
            ++n;
        }
    }
    return n != 0;
}

static bool QIintersect(const sptr<Shape> &shape, const v3f &org, RNG &rng)
{
    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        /* Random target in the shape bounding box */
        bounds3f box = shape->bounds();
        v3f dst = { Lerp(rng.f32(), box.lo.x, box.hi.x),
                    Lerp(rng.f32(), box.lo.y, box.hi.y),
                    Lerp(rng.f32(), box.lo.z, box.hi.z) };

        Ray ray = { org, dst - org };

        Interaction isect;
        if (shape->intersect(ray, isect) != shape->qIntersect(ray)) {
            ++n;
        }
    }
    return n != 0;
}

TEST_CASE("Sphere reintersection", "[sphere], [isect]")
{
    uptr<RNG> rng = RNG::create();
    sptr<Shape> sphere = RandomSphere(*rng);

    /* Random Ray origin */
    v3f org = RandomPoint(*rng);

    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        if (ReintersectShape(sphere, org, *rng)) {
            ++n;
        }
    }
    REQUIRE(n == 0);
}

TEST_CASE("Sphere qIntersect", "[sphere], [qisect]")
{
    uptr<RNG> rng = RNG::create();
    sptr<Shape> sphere = RandomSphere(*rng);

    v3f org = RandomPoint(*rng);

    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        if (QIintersect(sphere, org, *rng)) {
            ++n;
        }
    }
    REQUIRE(n == 0);
}

TEST_CASE("Mesh Reintersection", "[mesh], [isect]")
{
    uptr<RNG> rng = RNG::create();

    sptr<Shape> triangle = RandomTriangle(*rng);

    /* Random Ray origin */
    v3f org = RandomPoint(*rng);

    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        if (ReintersectShape(triangle, org, *rng)) {
            ++n;
        }
    }
    REQUIRE(n == 0);
}

TEST_CASE("Mesh qIntersect", "[mesh], [qisect]")
{
    uptr<RNG> rng = RNG::create();
    sptr<Shape> triangle = RandomTriangle(*rng);

    v3f org = RandomPoint(*rng);

    size_t n = 0;
    for (size_t i = 0; i < 1000; ++i) {
        if (QIintersect(triangle, org, *rng)) {
            ++n;
        }
    }
    REQUIRE(n == 0);
}
