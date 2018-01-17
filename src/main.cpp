#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <random>
#include <float.h>
#include <string>

#include "geometry.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "wqueue.hpp"
#include "event.hpp"
#include "bvh.hpp"
#include "imageio.h"
#include "sync.h"
#include "params.hpp"
#include "scene.hpp"

#define MAX_RECURSION_DEPTH 50

static v3f color(const sptr<ray> &r, const sptr<Primitive> &world, size_t depth)
{
    hit_record rec;

    if (world->hit(r, 0.001f, FLT_MAX, rec)) {
        v3f attenuation;
        sptr<ray> scattered;

        if (depth < MAX_RECURSION_DEPTH
            && rec.mat->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        }
        return { 0.0f, 0.0f, 0.0f };
    } else {
        v3f udir = r->direction().normalized();
        v3f white = { 1.0f, 1.0f, 1.0f };
        v3f blue = { 0.5f, 0.7f, 1.0f };

        float t = 0.5f * (udir.y + 1.0f);
        return Lerp(t, white, blue);
    }
}

static sptr<Primitive> random_scene()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<sptr<Primitive>> v;
    sptr<Shape> shape;
    sptr<Material> mat;

    sptr<Texture> tex = Texture::create_checker(Texture::create_color({ 0.2f, 0.3f, 0.2f }),
                                                Texture::create_color({ 0.9f, 0.9f, 0.9f }));


    shape = Sphere::create({ 0.0f, -1000.0f, 0.0f }, 1000.0f);
    mat = Lambertian::create(tex);
    v.push_back(Primitive::create(shape, mat));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = dist(mt);
            v3f center = {
                (float)a + 0.9f * dist(mt),
                0.2f,
                (float)b + 0.9f * dist(mt)
            };
            shape = Sphere::create(center, 0.2f);
            if ((center - v3f{ 4.0f, 0.2f, 0.0f }).length() > 0.9f) {
                if (choose_mat < 0.8f) {  // diffuse
                    v3f albedo = {
                        dist(mt) * dist(mt),
                        dist(mt) * dist(mt),
                        dist(mt) * dist(mt)
                    };
                    tex = Texture::create_color(albedo);
                    mat = Lambertian::create(tex);

                    v.push_back(Primitive::create(shape, mat));
                }
                else if (choose_mat < 0.95f) { // metal
                    v3f albedo = {
                        0.5f * (1 + dist(mt)),
                        0.5f * (1 + dist(mt)),
                        0.5f * (1 + dist(mt))
                    };
                    tex = Texture::create_color(albedo);
                    mat = Metal::create(tex, 0.5f * dist(mt));
                    v.push_back(Primitive::create(shape, mat));
                }
                else {  // glass
                    mat = Dielectric::create(1.5f);
                    v.push_back(Primitive::create(shape, mat));
                }
            }
        }
    }
    shape = Sphere::create({ 0.0f, 1.0f, 0.0 }, 1.0f);
    mat = Dielectric::create(1.5f);
    v.push_back(Primitive::create(shape, mat));

    shape = Sphere::create({ -4.0f, 1.0f, 0.0f }, 1.0f);
    mat = Lambertian::create(Texture::create_color({ 0.4f, 0.2f, 0.1f }));
    v.push_back(Primitive::create(shape, mat));

    shape = Sphere::create({ 4.0f, 1.0f, 0.0f }, 1.0f);
    mat = Metal::create(Texture::create_color({ 0.7f, 0.6f, 0.5f }), 0.0f);
    v.push_back(Primitive::create(shape, mat));

    return BVHNode::create(v);
}

struct _tile : Object {
    static sptr<_tile> create(rect_t r, uint8_t *dp, size_t bpr) {
        return std::make_shared<_tile>(r, dp, bpr);
    }
    _tile(rect_t r, uint8_t *dp, size_t bpr) : m_rect(r), m_dp(dp), m_bytes_per_row(bpr) { }
    ~_tile() { }

    rect_t   m_rect;
    uint8_t *m_dp;
    size_t   m_bytes_per_row;
};

struct _ctx : Object {
    static sptr<_ctx> create(const sptr<Camera> c,
                             sptr<Primitive> s,
                             uint32_t n,
                             v2u size) { return std::make_shared<_ctx>(c, s, n, size); }

    _ctx(const sptr<Camera> c, sptr<Primitive> s, uint32_t n, v2u size) {
        m_camera = c;
        m_scene = s;
        m_ns = n;
        m_img_size = size;
    }
    ~_ctx() { }

    sptr<Camera>    m_camera;
    sptr<Primitive> m_scene;
    uint32_t        m_ns;
    size_t          m_ntiles;
    v2u             m_img_size;
    sptr<Event>     m_event;
};

static void pixel_func(const sptr<Object> &obj, const sptr<Object> &arg)
{
    sptr<_ctx> ctx  = std::static_pointer_cast<_ctx>(obj);
    sptr<_tile> smp = std::static_pointer_cast<_tile>(arg);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    uint32_t nx = smp->m_rect.size.x;
    uint32_t ny = smp->m_rect.size.y;
    int32_t orgx = smp->m_rect.org.x;
    int32_t orgy = smp->m_rect.org.y;

    for (uint32_t i = 0; i < ny; i++) {
        uint8_t *dp = smp->m_dp + i * smp->m_bytes_per_row;

        for (uint32_t j = 0; j < nx; j++) {
            v3f c = { 0.0f, 0.0f, 0.0f };

            for (size_t k = 0; k < ctx->m_ns; k++) {
                float u = (orgx + (int32_t)j + dist(mt)) / float(ctx->m_img_size.x);
                float v = 1.0f - (float(orgy + (int32_t)i) - dist(mt)) / float(ctx->m_img_size.y);
                sptr<ray> r = ctx->m_camera->make_ray(u, v);

                c = c + color(r, ctx->m_scene, 0);
            }
            c = 1.0f / ctx->m_ns * c;
            /* Approx Gamma correction */
            c = { sqrtf(c.x), sqrtf(c.y), sqrtf(c.z) };

            dp[0] = (uint8_t)(255.99 * c.x);
            dp[1] = (uint8_t)(255.99 * c.y);
            dp[2] = (uint8_t)(255.99 * c.z);
            dp += 3;
        }
    }
    ctx->m_event->signal();
}

__attribute__((noreturn)) static void usage(const char *msg = nullptr)
{
    if (msg) {
        fprintf(stderr, "rt1w: %s\n\n", msg);
    }
    fprintf(stderr, R"(usage: rt1w [<options>] <scene file>
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
    char     file[256];
    uint32_t quality;
    uint32_t flags;
};

static void progress(const sptr<Object> &obj, const sptr<Object> &)
{
    static int32_t volatile progress = -1;

    int32_t done = sync_add_i32(&progress, 1);
    sptr<_ctx> ctx = std::static_pointer_cast<_ctx>(obj);

    float p = ctx ? (float)done / (float)ctx->m_ntiles * 100.0f : 0.0f;

    char buf[256];
    size_t offset = (size_t)snprintf(buf, 256, "\r%.1f%% [", p);
    int64_t n = lrint(floorf(p)) / 2;
    for (int32_t i = 0; i < 50; i++) {
        char c = i <= n ? '#' : ' ';
        snprintf(&buf[offset], 256 - offset, "%c", c);
        offset++;
    }
    snprintf(&buf[offset], 256- offset, "]");
    fprintf(stderr, "%s", buf);
    fflush(stderr);

    if ((uint32_t)done == ctx->m_ntiles) {
        fprintf(stderr, "\nDone!\n");
    }
}

int main(int argc, char *argv[])
{
    /* Process arguments */
    struct options options = { "\0", 0, 0 };

    if (argc == 1) {
        usage();
    }
    for (int i = 1; i < argc; i++) {
        if (char *c = strstr(argv[i], "--quality=")) {
            options.quality = (uint32_t)atoi(c + 10);
        }
        else if (char *cc = strstr(argv[i], "-quality=")) {
            options.quality = (uint32_t)atoi(cc + 9);
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
            strncpy(options.file, argv[i], 255);
        }
    }
    sptr<Scene> scene = Scene::create_json(options.file);

    v2u img_size = scene->camera()->resolution();
    uint32_t ns = 1 << options.quality;
    uint8_t *img = (uint8_t *)malloc(img_size.x * img_size.y * 3 * sizeof(*img));
    size_t bpr = img_size.x * 3 * sizeof(*img);

    /* Divide in tiles 32x32 */
    int32_t ntx = img_size.x / 32 + 1;
    int32_t nty = img_size.y / 32 + 1;

    std::vector<sptr<_tile>> tiles;
    for (int32_t i = 0; i < ntx; i++) {
        for (int32_t j = 0; j < nty; j++) {

            uint8_t *ptr = (uint8_t *)img + (uint32_t)j * 32 * bpr + (uint32_t)i * 32 * 3 * sizeof(*img);
            rect_t r;
            r.org.x = i * 32;
            r.org.y = j * 32;
            r.size.x = i < ntx - 1 ? 32 : 32 - ((uint32_t)ntx * 32 - img_size.x);
            r.size.y = j < nty - 1 ? 32 : 32 - ((uint32_t)nty * 32 - img_size.y);

            sptr<_tile> tile = _tile::create(r, ptr, bpr);
            tiles.push_back(tile);
        }
    }

    /* Actual rendering */
    sptr<_ctx> ctx = _ctx::create(scene->camera(), scene->primitive(), ns, img_size);
    ctx->m_ntiles = tiles.size();
    ctx->m_event = Event::create((int32_t)tiles.size());

    if (!options.flags & OPTION_QUIET) {
        progress(ctx, std::shared_ptr<Object>());
    }
    for (sptr<_tile> t : tiles) {
        sptr<Event> e = wqueue_execute(wqueue_get_queue(), pixel_func, ctx, t);
        e->notify(nullptr, progress, ctx, std::shared_ptr<Object>());
    }
    ctx->m_event->wait();

    std::string output = scene->options()->string("output");
    if (!output.empty()) {
        image_write_png(output.c_str(),
                        img_size.x,
                        img_size.y,
                        img,
                        bpr);
    }

    free(img);

    return 0;
}
