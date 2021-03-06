#include "rt1w/accelerator.hpp"
#include "rt1w/camera.hpp"
#include "rt1w/context.hpp"
#include "rt1w/denoise.hpp"
#include "rt1w/error.h"
#include "rt1w/event.hpp"
#include "rt1w/image.hpp"
#include "rt1w/imageio.h"
#include "rt1w/integrator.hpp"
#include "rt1w/params.hpp"
#include "rt1w/sampler.hpp"
#include "rt1w/scene.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

[[noreturn]] static void usage(const char *msg = nullptr)
{
    if (msg) {
        fprintf(stderr, "rt1w: %s\n\n", msg);
    }
    fprintf(stderr, R"(usage: rt1w [<options>] <scene file>
--help               Print this help text.
--quality=<num>      Size of the grid in which pixel are subdivided.
                     Set to zero by default, so only one ray per pixel.
--denoise            Apply a denoising step at the end of the rendering.
--albedo             Outputs the color on the first ray-shape hit.
--normals            Outputs the normals, remapped to [0, 1].
--quiet              Only prints error messages.
--verbose            Print more stuff.

)");
    exit(1);
}

enum {
    OPTION_QUIET = 1,
    OPTION_VERBOSE = 1 << 1,
    OPTION_DENOISE = 1 << 2,
    OPTION_NORMALS = 1 << 3,
    OPTION_ALBEDO = 1 << 4
};

struct options {
    char file[256];
    uint32_t quality;
    uint32_t flags;
};

static inline std::string RemoveExtensionJSON(const std::string &s)
{
    return s.substr(0, s.rfind(".json"));
}

int main(int argc, char *argv[])
{
    /* Process arguments */
    struct options options = { "\0", 1, 0 };

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
        else if (!strcmp(argv[i], "--denoise") || !strcmp(argv[i], "-denoise")) {
            options.flags |= OPTION_DENOISE;
        }
        else if (!strcmp(argv[i], "--albedo") || !strcmp(argv[i], "-albedo")) {
            options.flags |= OPTION_ALBEDO;
        }
        else if (!strcmp(argv[i], "--normals") || !strcmp(argv[i], "-normals")) {
            options.flags |= OPTION_NORMALS;
        }
        else if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-verbose")) {
            options.flags |= OPTION_VERBOSE;
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")
                 || !strcmp(argv[i], "-help")) {
            usage();
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
    std::string accelerator = Params::string(render->options(), "accelerator", "bvh");
    sptr<Primitive> accel = Accelerator::create(accelerator, render->primitives());

    /* Create Scene */
    sptr<Scene> scene = Scene::create(accel, render->lights());

    /* Get camera */
    sptr<Camera> camera = render->camera();

    /* Create integrator */
    uint32_t ns = options.quality;
    sptr<Sampler> sampler = Sampler::create(ns, ns, 4, true);

    std::string str = render->options()->string("integrator");
    sptr<Integrator> integrator = Integrator::create(str, sampler, 4);

    /* Get output filename */
    std::string output = Params::string(render->options(),
                                        "output",
                                        RemoveExtensionJSON(options.file));

    /* Create rendering context */
    sptr<Render> rdr = Render::create(scene, camera, integrator);

    /* Render and write out */
    sptr<Image> img = rdr->image();
    if (options.flags & OPTION_DENOISE) {
        img = Denoise(img, rdr->normals(), rdr->albedo());
    }
    img = Image::create(img, buffer_format_init(TYPE_UINT8, ORDER_RGB));

    buffer_t buf = img->buffer();
    image_write_png(std::string(output).append(".png").c_str(),
                    buf.rect.size.x,
                    buf.rect.size.y,
                    buf.data,
                    buf.bpr);

    if (options.flags & OPTION_ALBEDO) {
        auto albedo = Image::create(rdr->albedo(),
                                    buffer_format_init(TYPE_UINT8, ORDER_RGB));
        auto abuf = albedo->buffer();
        image_write_png(std::string(output).append("-albedo.png").c_str(),
                        abuf.rect.size.x,
                        abuf.rect.size.y,
                        abuf.data,
                        abuf.bpr);
    }
    if (options.flags & OPTION_NORMALS) {
        auto normals = Image::create(rdr->normals(),
                                     buffer_format_init(TYPE_UINT8, ORDER_RGB));
        auto nbuf = normals->buffer();
        image_write_png(std::string(output).append("-normals.png").c_str(),
                        nbuf.rect.size.x,
                        nbuf.rect.size.y,
                        nbuf.data,
                        nbuf.bpr);
    }

    return 0;
}
