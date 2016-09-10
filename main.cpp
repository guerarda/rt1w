#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec.hpp"
#include "ray.hpp"

float hit_sphere(const v3f &center, float r, const ray &ray)
{
    v3f oc = v3f_sub(ray.m_org, center);
    float a = v3f_dot(ray.m_dir, ray.m_dir);
    float b = 2 * v3f_dot(ray.m_dir, oc);
    float c = v3f_dot(oc, oc) - r * r;
    float delta = b * b - 4 * a * c;

    if (delta < 0.0f) {
        return -1.0f;
    } else {
        return (-b - sqrtf(delta)) / (2.0 * a);
    }
}

v3f color(const ray &ray)
{
    v3f center = { 0.0f, 0.0f, -1.0f };
    float t = hit_sphere(center, 0.5, ray);
    if (t > 0.0f) {
        v3f N = v3f_normalize(v3f_sub(ray.point_at_param(t), center));
        N.x += 1.0f;
        N.y += 1.0f;
        N.z += 1.0f;
        return v3f_smul(0.5f, N);
    }
    v3f udir = v3f_normalize(ray.m_dir);
    v3f white = { 1.0f, 1.0f, 1.0f };
    v3f blue = { 0.5f, 0.7f, 1.0f };

    t = 0.5f * (udir.y + 1.0f);
    return v3f_lerp(t, white, blue);
}

int main(__unused int argc, __unused  char *argv[])
{
    size_t nx = 800;
    size_t ny = 400;
    uint8_t *img = (uint8_t *)malloc(nx * ny * 3 * sizeof(*img));
    size_t bpr = nx * 3 * sizeof(*img);

    v3f bl = { -2.0f, -1.0f, -1.0f };
    v3f horizontal = { 4.0f, 0.0f, 0.0f };
    v3f vertical  = { 0.0f, 2.0f, 0.0f };
    v3f org = { 0.0f, 0.0f, 0.0f };

    for (size_t i = 0; i < ny; i++) {
            uint8_t *dp = (uint8_t *)((uint8_t *)img + i * bpr);
            for (size_t j = 0; j < nx; j++) {
                float u = (float)j / (float)nx;
                float v = (float)(ny - i) / (float)ny;
                ray r = ray(org, v3f_add(v3f_add(bl, v3f_smul(u, horizontal)),
                                         v3f_smul(v, vertical)));
                v3f c = color(r);

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
