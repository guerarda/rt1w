#include "catch.hpp"

#include "camera.hpp"
#include "ray.hpp"

#include <random>

static CameraSample randomCameraSample(const v2u &res)
{
    static std::random_device rd;
    static std::mt19937 __prng(rd());
    static std::uniform_real_distribution<float> dist(0.0, 1.0f);

    return { { res.x * dist(__prng), res.y * dist(__prng) },
             { dist(__prng), dist(__prng) } };
}

TEST_CASE("Perspective Camera")
{
    v3f pos = { 5.0, 4.0, 3.0 };
    v3f lookat = { 1.0, 1.0, 0.0 };
    v3f up = { 0.0, 1.0, 0.0 };
    v2u res = { 1920, 1080 };
    v2f screen = { 1920.0f / 1080.0f, 2.0f };
    float fov = 20.0f;
    float aperture = 0.1f;
    float fdist = 10.0f;
    float zNear = -0.1f;
    float zFar = -100.0f;

    auto camera = PerspectiveCamera::create(pos,
                                            lookat,
                                            up,
                                            res,
                                            screen,
                                            fov,
                                            aperture,
                                            fdist,
                                            zNear,
                                            zFar);

    SECTION("Resolution")
    {
        v2u r = camera->resolution();
        CHECK(r.x == res.x);
        CHECK(r.y == res.y);
    }
    SECTION("Ray origin")
    {
        size_t n = 0;
        for (size_t i = 0; i < 10000; i++) {
            auto cs = randomCameraSample(res);
            auto ray = camera->generateRay(cs);
            auto diff = ray->origin() - pos;

            if (v2f{ diff.x, diff.y }.length() > aperture) {
                n++;
            }
        }
        CHECK(n == 0);
    }
    SECTION("Rays are normalized")
    {
        size_t n = 0;
        for (size_t i = 0; i < 10000; i++) {
            auto ray = camera->generateRay(randomCameraSample(res));
            if (!(ray->direction().length() == Approx(1.0))) {
                n++;
            }
        }
        CHECK(n == 0);
    }
    SECTION("Rays points toward lookat")
    {
        size_t n = 0;
        v3f d = lookat - pos;
        for (size_t i = 0; i < 10000; i++) {
            auto ray = camera->generateRay(randomCameraSample(res));
            if (!(Dot(d, ray->direction()) > 0.0)) {
                n++;
            }
        }
        CHECK(n == 0);
    }
}

TEST_CASE("Orthographic Camera")
{
    v3f pos = { 5.0, 4.0, 3.0 };
    v3f lookat = { 1.0, 1.0, 0.0 };
    v3f up = { 0.0, 1.0, 0.0 };
    v2u res = { 1920, 1080 };
    v2f screen = { 1920.0f / 1080.0f, 2.0f };
    float aperture = 0.1f;
    float fdist = 10.0f;
    float zNear = -0.1f;
    float zFar = -100.0f;

    auto camera = OrthographicCamera::create(pos,
                                             lookat,
                                             up,
                                             res,
                                             screen,
                                             aperture,
                                             fdist,
                                             zNear,
                                             zFar);

    SECTION("Resolution")
    {
        v2u r = camera->resolution();
        CHECK(r.x == res.x);
        CHECK(r.y == res.y);
    }
    SECTION("Ray origin")
    {
        size_t n = 0;
        for (size_t i = 0; i < 10000; i++) {
            auto cs = randomCameraSample(res);
            auto ray = camera->generateRay(cs);
            auto diff = ray->origin() - pos;

            if (v2f{ diff.x, diff.y }.length() > aperture) {
                n++;
            }
        }
        CHECK(n == 0);
    }
    SECTION("Rays are normalized")
    {
        size_t n = 0;
        for (size_t i = 0; i < 10000; i++) {
            auto ray = camera->generateRay(randomCameraSample(res));
            if (!(ray->direction().length() == Approx(1.0))) {
                n++;
            }
        }
        CHECK(n == 0);
    }
    SECTION("Rays points toward lookat")
    {
        size_t n = 0;
        v3f d = lookat - pos;
        for (size_t i = 0; i < 10000; i++) {
            auto ray = camera->generateRay(randomCameraSample(res));
            if (!(Dot(d, ray->direction()) > 0.0)) {
                n++;
            }
        }
        CHECK(n == 0);
    }
}
