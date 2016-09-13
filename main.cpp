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

v3f color(const sptr<ray> &r, const sptr<hitable> &world, size_t depth)
{
    hit_record rec;

    if (world->hit(r, 0.001f, FLT_MAX, rec)) {
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

sptr<hitable> random_scene()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    sptr<hitable> list[500];
    list[0] = sphere::create({ 0.0f, -1000.0f, 0.0f },
                             1000.0f,
                             lambertian::create({0.5, 0.5, 0.5}));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = dist(mt);
            v3f center = {
                (float)a + 0.9f * dist(mt),
                0.2f,
                (float)b + 0.9f * dist(mt)
            };
            if ((v3f_norm(v3f_sub(center,{ 4.0f, 0.2f, 0.0f }))) > 0.9f) {
                if (choose_mat < 0.8f) {  // diffuse
                    v3f albedo = {
                        dist(mt) * dist(mt),
                        dist(mt) * dist(mt),
                        dist(mt) * dist(mt)
                    };
                    list[i++] = sphere::create(center,
                                               0.2f,
                                               lambertian::create(albedo));
                }
                else if (choose_mat < 0.95f) { // metal
                    v3f albedo = {
                        0.5f * (1 + dist(mt)),
                        0.5f * (1 + dist(mt)),
                        0.5f * (1 + dist(mt))
                    };
                    list[i++] = sphere::create(center,
                                               0.2f,
                                               metal::create(albedo, 0.5f * dist(mt)));
                }
                else {  // glass
                    list[i++] = sphere::create(center, 0.2, dielectric::create(1.5));
                }
            }
        }
    }

    list[i++] = sphere::create({ 0.0f, 1.0f, 0.0 },
                               1.0f,
                               dielectric::create(1.5f));
    list[i++] = sphere::create({ -4.0f, 1.0f, 0.0f },
                               1.0f,
                               lambertian::create({ 0.4f, 0.2f, 0.1f }));
    list[i++] = sphere::create({ 4.0f, 1.0f, 0.0f },
                               1.0f, metal::create({ 0.7f, 0.6f, 0.5f },
                                                   0.0f));

    return hitable_list::create(i, list);
}

static void usage(const char *msg = nullptr)
{
    if (msg) {
        fprintf(stderr, "rt1w: %s\n\n", msg);
    }
    fprintf(stderr, R"(usage: rt1w [<options>] <outfile>
--help               Print this help text.
--quality=<num>      log2 of the number of ray traced for each pixel.
                     Set to zero by default, so only one ray per pixel.
--quiet              Only prints error messages.
--verbose            Print more stuff.

)");
    exit(1);
}

enum {
    OPTION_QUIET   = 1,
    OPTION_VERBOSE = 1<<1
};

struct options {
    char     outfile[256];
    uint32_t quality;
    uint32_t flags;
};

int main(int argc, char *argv[])
{
    /* Process arguments */
    struct options options = { "\0", 0, 0 };

    if (argc == 1) {
        usage();
    }
    for (int i = 1; i < argc; i++) {
        if (char * c = strstr(argv[i], "--quality=")) {
            options.quality = atoi(c + 10);
        }
        else if (char * c = strstr(argv[i], "-quality=")) {
            options.quality = atoi(c + 9);
        }
        else if (   !strcmp(argv[i], "--quiet")
                 || !strcmp(argv[i], "-quiet")) {
            options.flags |= OPTION_QUIET;
        }
        else if (   !strcmp(argv[i], "--verbose")
                 || !strcmp(argv[i], "-verbose")) {
            options.flags |= OPTION_VERBOSE;
        }
        else if (   !strcmp(argv[i], "--help")
                 || !strcmp(argv[i], "-h")
                 || !strcmp(argv[i], "-help")) {
            usage();
            return 0;
        } else {
            strncpy(options.outfile, argv[i], 255);
        }
    }
    size_t nx = 800;
    size_t ny = 400;
    size_t ns = 1 << options.quality;
    uint8_t *img = (uint8_t *)malloc(nx * ny * 3 * sizeof(*img));
    size_t bpr = nx * 3 * sizeof(*img);

    v3f eye = { 13.0f, 2.0f, 3.0f };
    v3f lookat = { 0.0f, 0.0f, 0.0f };
    v3f up = { 0.0f, 1.0f, 0.0f };
    float aspect = (float)nx / (float)ny;
    float aperture = 0.1f;
    float focus_dist = 10.0f;

    sptr<camera> camera = camera::create(eye, lookat, up, 20.0f, aspect,
                                         aperture, focus_dist);
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    sptr<hitable> scene = random_scene();
    for (size_t i = 0; i < ny; i++) {
            uint8_t *dp = (uint8_t *)((uint8_t *)img + i * bpr);

            for (size_t j = 0; j < nx; j++) {
                v3f c = { 0.0f, 0.0f, 0.0f };

                for (size_t k = 0; k < ns; k++) {
                    float u = (float)(j + dist(mt)) / (float)nx;
                    float v = (float)(ny - (i - dist(mt))) / (float)ny;
                    sptr<ray> r = camera->make_ray(u, v);

                    c = v3f_add(c, color(r, scene, 0));
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
    stbi_write_png(options.outfile, nx, ny, 3, img, bpr);
    free(img);

    return 0;
}
