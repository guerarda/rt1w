#include <stdlib.h>
#include <stdint.h>
#include <random>

#include "vec.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "hitablelist.hpp"
#include "camera.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

v3f color(const sptr<ray> &ray, const sptr<hitable> &world)
{
    hit_record rec;

    if (world->hit(ray, 0.0f, MAXFLOAT, rec)) {
        v3f N = v3f_normalize(rec.normal);
        N.x += 1.0f;
        N.y += 1.0f;
        N.z += 1.0f;
        return v3f_smul(0.5f, N);
    }
    v3f udir = v3f_normalize(ray->direction());
    v3f white = { 1.0f, 1.0f, 1.0f };
    v3f blue = { 0.5f, 0.7f, 1.0f };

    float t = 0.5f * (udir.y + 1.0f);
    return v3f_lerp(t, white, blue);
}

int main(__unused int argc, __unused  char *argv[])
{
    size_t nx = 800;
    size_t ny = 400;
    size_t ns = 1;
    uint8_t *img = (uint8_t *)malloc(nx * ny * 3 * sizeof(*img));
    size_t bpr = nx * 3 * sizeof(*img);

    v3f bl = { -2.0f, -1.0f, -1.0f };
    v3f horizontal = { 4.0f, 0.0f, 0.0f };
    v3f vertical  = { 0.0f, 2.0f, 0.0f };
    v3f org = { 0.0f, 0.0f, 0.0f };

    sptr<camera> camera = camera::create(bl, horizontal, vertical, org);
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    sptr<hitable> list[2];
    list[0] = sphere::create({ 0.0f, 0.0f, -1.0f }, 0.5f);
    list[1] = sphere::create({ 0.0f, -100.5f, -1.0f }, 100.0f);

    sptr<hitable_list> world = hitable_list::create(2, list);

    sptr<sphere> sphere = sphere::create({ 0.0f, 0.0f, -1.0f }, 0.5f);

    for (size_t i = 0; i < ny; i++) {
            uint8_t *dp = (uint8_t *)((uint8_t *)img + i * bpr);
            for (size_t j = 0; j < nx; j++) {

                v3f c = { 0.0f, 0.0f, 0.0f };

                for (size_t k= 0; k < ns; k++) {
                    float u = (float)(j + dist(mt)) / (float)nx;
                    float v = (float)(ny - (i - dist(mt))) / (float)ny;
                    sptr<ray> r = camera->make_ray(u, v);

                    c = v3f_add(c, color(r, world));
                }
                c = v3f_smul(1.0f / ns, c);

                dp[0] = (int32_t)(255.99 * c.x);
                dp[1] = (int32_t)(255.99 * c.y);
                dp[2] = (int32_t)(255.99 * c.z);
                dp += 3;
        }
    }
    stbi_write_png("rt1w.png", nx, ny, 3, img, bpr);
    free(img);

    return 0;
}
