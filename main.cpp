#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec.h"
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

    for (size_t i = 0; i < ny; i++) {
            uint8_t *dp = (uint8_t *)((uint8_t *)img + i * bpr);
            for (size_t j = 0; j < nx; j++) {
                float r = (float)j / (float)nx;
                float g = (float)i / (float)ny;
                float b = 0.2f;

                dp[0] = (int32_t)(255.99 * r);
                dp[1] = (int32_t)(255.99 * g);
                dp[2] = (int32_t)(255.99 * b);
                dp += 3;
        }
    }
    stbi_write_png("rt1w.png", nx, ny, 3, img, bpr);
    free(img);
}
