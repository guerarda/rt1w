#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <random>
#include <float.h>

#include "vec.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "hitablelist.hpp"
#include "camera.hpp"
#include "material.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MAX_RECURSION_DEPTH 50

static void print_info_str()
{
    fprintf(stderr, "Usage: rt1f output_path ray_count\n");
}

v3f color(const sptr<ray> &r, const sptr<hitable> &world, size_t depth)
{
    hit_record rec;

    if (world->hit(r, 0.00001f, FLT_MAX, rec)) {
        v3f attenuation;
        sptr<ray> scattered;

        if (depth < MAX_RECURSION_DEPTH
            && rec.mat->scatter(r, rec, attenuation, scattered)) {
            return v3f_vmul(attenuation, color(scattered, world, depth + 1));
        } else {
            return { 0.0f, 0.0f, 0.0f };
        }
    } else {
        v3f udir = v3f_normalize(r->direction());
        v3f white = { 1.0f, 1.0f, 1.0f };
        v3f blue = { 0.5f, 0.7f, 1.0f };

        float t = 0.5f * (udir.y + 1.0f);
        return v3f_lerp(t, white, blue);
    }
}

int main(int argc, char *argv[])
{
    const char *arg1 = nullptr;
    int arg2 = 0;

    if (argc == 1) {
        arg1 = "rt1w.png";
        arg2 = 1;
    }
    else if (argc == 3) {
        arg1 = argv[1];
        arg2 = atoi(argv[2]);
    } else {
        print_info_str();
        return 0;
    }

    size_t nx = 800;
    size_t ny = 400;
    size_t ns = arg2;
    uint8_t *img = (uint8_t *)malloc(nx * ny * 3 * sizeof(*img));
    size_t bpr = nx * 3 * sizeof(*img);

    v3f eye = { -2.0f, 2.0f, 1.0f };
    v3f lookat = { 0.0f, 0.0f, -1.0f };
    v3f up = { 0.0f, 1.0f, 0.0f };
    float aspect = (float)nx / (float)ny;

    sptr<camera> camera = camera::create(eye, lookat, up, 30.0f, aspect);
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    sptr<material> soil = lambertian::create({ 0.8f, 0.8f, 0.0f });
    sptr<material> mat = lambertian::create({ 0.1f, 0.2f, 0.5f });
    sptr<material> metal = metal::create({ 0.8f, 0.6f, 0.2f }, 0.3f);
    sptr<material> glass = dielectric::create(1.5f);

    sptr<hitable> list[4];
    list[0] = sphere::create({ 0.0f, 0.0f, -1.0f }, 0.5f, mat);
    list[1] = sphere::create({ 0.0f, -100.5f, -1.0f }, 100.0f, soil);
    list[2] = sphere::create({ 1.0f, 0.0f, -1.0f }, 0.5f, metal);
    list[3] = sphere::create({ -1.0f, 0.0f, -1.0f }, 0.5f, glass);

    sptr<hitable_list> world = hitable_list::create(4, list);

    for (size_t i = 0; i < ny; i++) {
            uint8_t *dp = (uint8_t *)((uint8_t *)img + i * bpr);

            for (size_t j = 0; j < nx; j++) {
                v3f c = { 0.0f, 0.0f, 0.0f };

                for (size_t k = 0; k < ns; k++) {
                    float u = (float)(j + dist(mt)) / (float)nx;
                    float v = (float)(ny - (i - dist(mt))) / (float)ny;
                    sptr<ray> r = camera->make_ray(u, v);

                    c = v3f_add(c, color(r, world, 0));
                }
                c = v3f_smul(1.0f / ns, c);

                /* Approx Gamma correction */
                c = { sqrtf(c.x), sqrtf(c.y), sqrtf(c.z) };

                dp[0] = (int32_t)(255.99 * c.x);
                dp[1] = (int32_t)(255.99 * c.y);
                dp[2] = (int32_t)(255.99 * c.z);
                dp += 3;
        }
    }
    stbi_write_png(arg1, nx, ny, 3, img, bpr);
    free(img);

    return 0;
}
