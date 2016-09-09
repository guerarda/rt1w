#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec.hpp"
#include "ray.hpp"

v3f color(const ray &ray)
{
    v3f udir = v3f_normalize(ray.m_dir);
    float t = 0.5f * (udir.y + 1.0f);
    v3f white = { 1.0f, 1.0f, 1.0f };
    v3f blue = { 0.5f, 0.7f, 1.0f };
    return v3f_lerp(t, white, blue);
}

int main(__unused int argc, __unused  char *argv[])
{
    size_t nx = 200;
    size_t ny = 100;
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
                ray r = ray(org, v3f_add(v3f_add(bl, v3f_muls(u, horizontal)),
                                         v3f_muls(v, vertical)));
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
