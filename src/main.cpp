#include "bvh.hpp"
#include "camera.hpp"
#include "context.hpp"
#include "error.h"
#include "event.hpp"
#include "image.hpp"
#include "imageio.h"
#include "integrator.hpp"
#include "params.hpp"
#include "sampler.hpp"
#include "scene.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

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

enum { OPTION_QUIET = 1, OPTION_VERBOSE = 1 << 1 };

struct options {
    char file[256];
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
        if (char *c = strstr(argv[i], "--quality=")) {
            options.quality = (uint32_t)atoi(c + 10);
        }
        else if (char *cc = strstr(argv[i], "-quality=")) {
            options.quality = (uint32_t)atoi(cc + 9);
        }
        else if (!strcmp(argv[i], "--quiet") || !strcmp(argv[i], "-quiet")) {
            options.flags |= OPTION_QUIET;
        }
        else if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-verbose")) {
            options.flags |= OPTION_VERBOSE;
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")
                 || !strcmp(argv[i], "-help")) {
            usage();
            return 0;
        }
        else {
            strncpy(options.file, argv[i], 255);
        }
    }
    sptr<RenderDescription> render = RenderDescription::load(options.file);

    DIE_IF(!render, "Nothing to render");

    WARNING_IF(render->primitives().empty(), "Scene has no primitive");
    WARNING_IF(render->lights().empty(), "Scene has no light");

    /* Create BVH */
    sptr<Primitive> bvh = BVHAccelerator::create(render->primitives());

    /* Create Scene */
    sptr<Scene> scene = Scene::create(bvh, render->lights());

    /* Get camera */
    sptr<Camera> camera = render->camera();

    /* Create integrator */
    uint32_t ns = options.quality;
    sptr<Sampler> sampler = Sampler::create(ns, ns, 4, true);

    sptr<Integrator> integrator;
    std::string str = render->options()->string("integrator");
    if (str == "path") {
        integrator = PathIntegrator::create(sampler, 4);
    }
    else {
        integrator = Integrator::create(sampler, 4);
    }

    /* Create rendering context */
    sptr<Render> rdr =
        Render::create(scene, camera, integrator, { TYPE_UINT8, ORDER_RGB });

    buffer_t buf = rdr->image()->buffer();

    std::string output = render->options()->string("output");
    if (!output.empty()) {
        image_write_png(output.c_str(),
                        buf.rect.size.x,
                        buf.rect.size.y,
                        buf.data,
                        buf.bpr);
    }
    return 0;
}
