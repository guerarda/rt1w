#include "context.hpp"

#include "camera.hpp"
#include "event.hpp"
#include "image.hpp"
#include "integrator.hpp"
#include "ray.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "workq.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

constexpr uint32_t TileSize = 32;

struct ImageTile : Object {
    static sptr<ImageTile> create(const buffer_t &buffer)
    {
        return std::make_shared<ImageTile>(buffer);
    }
    ImageTile(const buffer_t &buffer) : m_buffer(buffer) {}

    buffer_t m_buffer;
};

struct RenderingContext : Object, std::enable_shared_from_this<RenderingContext> {
    RenderingContext(const sptr<Scene> &scene,
                     const sptr<Camera> &camera,
                     const sptr<Integrator> &integrator,
                     const RenderOptions &options,
                     workq_func func) :
        m_scene(scene),
        m_camera(camera),
        m_integrator(integrator),
        m_options(options),
        m_func(func)
    {}
    sptr<Event> schedule();
    buffer_t buffer();

    sptr<Scene> m_scene;
    sptr<Camera> m_camera;
    sptr<Integrator> m_integrator;
    sptr<Event> m_event;
    buffer_t m_buffer;
    RenderOptions m_options;
    size_t m_ntiles;
    workq_func m_func;
};

static void progress(const sptr<Object> &, const sptr<Object> &);
static void render_tile(const sptr<Object> &, const sptr<Object> &);

sptr<Event> RenderingContext::schedule()
{
    v2u size = m_camera->resolution();
    m_buffer.format = buffer_format_init(m_options.type, m_options.order);
    m_buffer.rect = { { 0, 0 }, { size.x, size.y } };

    m_buffer.bpr = size.x * m_buffer.format.size;

    ASSERT(m_buffer.bpr > 0);

    m_buffer.data = malloc(size.y * m_buffer.bpr);

    /* Divide in tiles */
    uint32_t ntx = size.x / TileSize + 1;
    uint32_t nty = size.y / TileSize + 1;

    std::vector<sptr<ImageTile>> tiles;
    for (size_t i = 0; i < ntx; ++i) {
        for (size_t j = 0; j < nty; ++j) {
            rect_t r;
            r.org.x = (int32_t)(i * TileSize);
            r.org.y = (int32_t)(j * TileSize);
            r.size.x = i < ntx - 1 ? TileSize : TileSize - (ntx * TileSize - size.x);
            r.size.y = j < nty - 1 ? TileSize : TileSize - (nty * TileSize - size.y);

            sptr<ImageTile> tile = ImageTile::create(
                { m_buffer.data, m_buffer.bpr, r, m_buffer.format });
            tiles.push_back(tile);
        }
    }
    m_event = Event::create((int32_t)tiles.size());
    m_ntiles = tiles.size();

    for (const auto &t : tiles) {
        sptr<RenderingContext> ctx = shared_from_this();
        sptr<Event> e = workq_execute(workq_get_queue(), m_func, ctx, t);
        e->notify(nullptr, progress, ctx, sptr<Object>());
    }
    return m_event;
}

buffer_t RenderingContext::buffer()
{
    schedule()->wait();
    return m_buffer;
}

#pragma mark - Image from RenderingContext

struct ImageFromCtx : Image {
    static sptr<Image> create(const sptr<RenderingContext> &ctx)
    {
        return std::make_shared<ImageFromCtx>(ctx);
    }

    ImageFromCtx(const sptr<RenderingContext> &ctx) : m_ctx(ctx) {}

    buffer_t buffer() override { return m_ctx->buffer(); }
    v2u size() const override { return m_ctx->m_camera->resolution(); }

    sptr<RenderingContext> m_ctx;
};

#pragma mark - Render

struct _Render : Render {
    _Render(const sptr<Scene> &scene,
            const sptr<Camera> &camera,
            const sptr<Integrator> &integrator,
            const RenderOptions &options) :
        m_scene(scene),
        m_camera(camera),
        m_integrator(integrator),
        m_options(options)
    {
        ASSERT(options.type == TYPE_UINT8 || options.type == TYPE_UINT16
               || options.type == TYPE_FLOAT32);
        ASSERT(options.order == ORDER_RGB);
    }

    sptr<Image> image() const override
    {
        return ImageFromCtx::create(std::make_shared<RenderingContext>(m_scene,
                                                                       m_camera,
                                                                       m_integrator,
                                                                       m_options,
                                                                       render_tile));
    }

    sptr<Scene> m_scene;
    sptr<Camera> m_camera;
    sptr<Integrator> m_integrator;
    RenderOptions m_options;
};

#pragma mark - Static constructor

sptr<Render> Render::create(const sptr<Scene> &scene,
                            const sptr<Camera> &camera,
                            const sptr<Integrator> &integrator,
                            const RenderOptions &options)
{
    return std::make_shared<_Render>(scene, camera, integrator, options);
}

#pragma mark - Static functions

static void write_pixel(void *dst, const v3f c, const buffer_format &fmt)
{
    ASSERT(dst);
    ASSERT(fmt.type > TYPE_VOID);

    switch (fmt.type) {
    case TYPE_UINT8:
        ((uint8_t *)dst)[0] = (uint8_t)std::rint(255.f * c.x);
        ((uint8_t *)dst)[1] = (uint8_t)std::rint(255.f * c.y);
        ((uint8_t *)dst)[2] = (uint8_t)std::rint(255.f * c.z);
        break;
    case TYPE_UINT16:
        ((uint16_t *)dst)[0] = (uint16_t)std::rint(65535.f * c.x);
        ((uint16_t *)dst)[1] = (uint16_t)std::rint(65535.f * c.y);
        ((uint16_t *)dst)[2] = (uint16_t)std::rint(65535.f * c.z);
        break;
    case TYPE_FLOAT32:
        ((float *)dst)[0] = c.x;
        ((float *)dst)[1] = c.y;
        ((float *)dst)[2] = c.z;
        break;
    default: break;
    }
}

static void render_tile(const sptr<Object> &obj, const sptr<Object> &arg)
{
    sptr<RenderingContext> ctx = std::static_pointer_cast<RenderingContext>(obj);
    sptr<ImageTile> tile = std::static_pointer_cast<ImageTile>(arg);

    buffer_t b = tile->m_buffer;

    int32_t orgx = b.rect.org.x;
    int32_t orgy = b.rect.org.y;
    int32_t maxx = orgx + (int32_t)b.rect.size.x;
    int32_t maxy = orgy + (int32_t)b.rect.size.y;

    sptr<Sampler> sampler = ctx->m_integrator->sampler()->clone();
    float ns_inv = 1.0f / sampler->samplesPerPixel();

    for (int32_t y = orgy; y < maxy; ++y) {
        uint8_t *dp = (uint8_t *)b.data + (size_t)y * b.bpr
                      + (size_t)orgx * b.format.size;
        for (int32_t x = orgx; x < maxx; ++x) {
            v3f c = { .0f, .0f, .0f };
            sampler->startPixel({ x, y });
            do {
                CameraSample cs = sampler->cameraSample();
                Ray r = ctx->m_camera->generateRay(cs);
                c = c + ctx->m_integrator->Li(r, ctx->m_scene, sampler, 0);
            } while (sampler->startNextSample());

            c *= ns_inv;

            /* Approx Gamma correction */
            c = { std::min(1.f, std::sqrt(c.x)),
                  std::min(1.f, std::sqrt(c.y)),
                  std::min(1.f, std::sqrt(c.z)) };
            write_pixel((void *)dp, c, b.format);
            dp += b.format.size;
        }
    }
    ctx->m_event->signal();
}

static void progress(const sptr<Object> &obj, const sptr<Object> &)
{
    static std::atomic<int32_t> progress(1);

    int32_t done = progress.fetch_add(1, std::memory_order_relaxed);
    sptr<RenderingContext> ctx = std::static_pointer_cast<RenderingContext>(obj);
    ASSERT(ctx);

    float p = (float)done / ctx->m_ntiles * 100.f;
    char buf[256];
    auto offset = (size_t)snprintf(buf, 256, "\r%.1f%% [", p);
    int64_t n = lrint(floor(p)) / 2;
    for (int32_t i = 0; i < 50; i++) {
        char c = i <= n ? '#' : ' ';
        snprintf(&buf[offset], 256 - offset, "%c", c);
        offset++;
    }
    snprintf(&buf[offset], 256 - offset, "]");
    fprintf(stderr, "%s", buf);
    if ((uint32_t)done == ctx->m_ntiles) {
        fprintf(stderr, "\nDone!\n");
    }
    fflush(stderr);
}
