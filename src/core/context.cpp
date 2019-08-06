#include "rt1w/context.hpp"

#include "rt1w/camera.hpp"
#include "rt1w/event.hpp"
#include "rt1w/image.hpp"
#include "rt1w/integrator.hpp"
#include "rt1w/ray.hpp"
#include "rt1w/sampler.hpp"
#include "rt1w/scene.hpp"
#include "rt1w/workq.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

constexpr uint32_t TileSize = 32;

static void Progress(const sptr<Object> &, const sptr<Object> &);
static void RenderTile(const sptr<Object> &, const sptr<Object> &);

struct ImageTile : Object {
    ImageTile(const rect_t &rect) : m_rect(rect) {}

    rect_t m_rect;
};

struct RenderingContext : Object, std::enable_shared_from_this<RenderingContext> {
    RenderingContext(const sptr<Scene> &scene,
                     const sptr<Camera> &camera,
                     const sptr<Integrator> &integrator,
                     workq_func func) :
        m_scene(scene),
        m_camera(camera),
        m_integrator(integrator),
        m_func(func),
        m_scheduled(0)
    {}

    ~RenderingContext() override
    {
        if (m_image.data) {
            free(m_image.data);
        }
        if (m_normals.data) {
            free(m_normals.data);
        }
        if (m_albedo.data) {
            free(m_albedo.data);
        }
    }
    sptr<Event> schedule();

    sptr<Scene> m_scene;
    sptr<Camera> m_camera;
    sptr<Integrator> m_integrator;

    size_t m_ntiles;
    workq_func m_func;

    buffer_t m_image;
    buffer_t m_normals;
    buffer_t m_albedo;

    sptr<Event> m_event;
    std::atomic<int32_t> m_scheduled;
};

sptr<Event> RenderingContext::schedule()
{
    while (m_scheduled.load() != 1) {
        int32_t expected = 0;
        if (m_scheduled.compare_exchange_strong(expected, -1)) {
            buffer_format_t fmt = buffer_format_init(TYPE_FLOAT32, ORDER_RGB);
            v2u size = m_camera->resolution();
            size_t bpr = size.x * fmt.size;
            size_t bsize = size.y * bpr;

            ASSERT(bpr > 0);
            ASSERT(bsize > 0);

            m_image = { malloc(bsize), bpr, { { 0, 0 }, { size.x, size.y } }, fmt };
            m_normals = { malloc(bsize), bpr, { { 0, 0 }, { size.x, size.y } }, fmt };
            m_albedo = { malloc(bsize), bpr, { { 0, 0 }, { size.x, size.y } }, fmt };

            /* Divide in tiles */
            uint32_t ntx = size.x / TileSize + 1;
            uint32_t nty = size.y / TileSize + 1;

            std::vector<sptr<ImageTile>> tiles;
            for (size_t i = 0; i < ntx; ++i) {
                for (size_t j = 0; j < nty; ++j) {
                    rect_t r;
                    r.org.x = (int32_t)(i * TileSize);
                    r.org.y = (int32_t)(j * TileSize);
                    r.size.x = i < ntx - 1 ? TileSize
                                           : TileSize - (ntx * TileSize - size.x);
                    r.size.y = j < nty - 1 ? TileSize
                                           : TileSize - (nty * TileSize - size.y);

                    tiles.emplace_back(std::make_shared<ImageTile>(r));
                }
            }
            m_event = Event::create((int32_t)tiles.size());
            m_ntiles = tiles.size();

            sptr<RenderingContext> ctx = shared_from_this();
            for (const auto &t : tiles) {
                sptr<Event> e = workq_execute(workq_get_queue(), m_func, ctx, t);
                e->notify(nullptr, Progress, ctx, sptr<Object>());
            }
            m_scheduled.store(1);
        }
    }
    return m_event;
}

#pragma mark - Image from RenderingContext

struct ImageFromCtx : Image {
    static sptr<Image> create(const sptr<RenderingContext> &ctx, buffer_t *b)
    {
        return std::make_shared<ImageFromCtx>(ctx, b);
    }

    ImageFromCtx(const sptr<RenderingContext> &ctx, buffer_t *b) : m_ctx(ctx), m_buffer(b)
    {}

    buffer_t buffer() override
    {
        schedule()->wait();
        return *m_buffer;
    }
    sptr<Event> schedule() override { return m_ctx->schedule(); }
    v2u size() const override { return m_ctx->m_camera->resolution(); }

    sptr<RenderingContext> m_ctx;
    buffer_t *m_buffer;
};

#pragma mark - Render

struct _Render : Render {
    _Render(const sptr<Scene> &scene,
            const sptr<Camera> &camera,
            const sptr<Integrator> &integrator) :
        m_ctx(std::make_shared<RenderingContext>(scene, camera, integrator, RenderTile)),
        m_image(ImageFromCtx::create(m_ctx, &m_ctx->m_image)),
        m_normals(ImageFromCtx::create(m_ctx, &m_ctx->m_normals)),
        m_albedo(ImageFromCtx::create(m_ctx, &m_ctx->m_albedo))
    {}

    sptr<Image> image() const override { return m_image; }
    sptr<Image> normals() const override { return m_normals; }
    sptr<Image> albedo() const override { return m_albedo; }

    sptr<RenderingContext> m_ctx;
    sptr<Image> m_image;
    sptr<Image> m_normals;
    sptr<Image> m_albedo;
};

#pragma mark - Static constructor

sptr<Render> Render::create(const sptr<Scene> &scene,
                            const sptr<Camera> &camera,
                            const sptr<Integrator> &integrator)
{
    return std::make_shared<_Render>(scene, camera, integrator);
}

#pragma mark - Static functions

static inline uint8_t *PixelPtr(const buffer_t &b, int32_t x, int32_t y)
{
    return (uint8_t *)b.data + y * (ptrdiff_t)b.bpr + x * (ptrdiff_t)b.format.size;
}

static inline v3f ApproxGammaCorrection(const v3f &c)
{
    return { std::min(1.f, std::sqrt(c.x)),
             std::min(1.f, std::sqrt(c.y)),
             std::min(1.f, std::sqrt(c.z)) };
}
static void RenderTile(const sptr<Object> &obj, const sptr<Object> &arg)
{
    sptr<RenderingContext> ctx = std::static_pointer_cast<RenderingContext>(obj);
    sptr<ImageTile> tile = std::static_pointer_cast<ImageTile>(arg);

    rect_t rect = tile->m_rect;
    int32_t orgx = rect.org.x;
    int32_t orgy = rect.org.y;
    int32_t maxx = orgx + (int32_t)rect.size.x;
    int32_t maxy = orgy + (int32_t)rect.size.y;

    buffer_t image = ctx->m_image;
    buffer_t normals = ctx->m_normals;
    buffer_t albedo = ctx->m_albedo;

    sptr<Sampler> sampler = ctx->m_integrator->sampler()->clone();
    float ns_inv = 1.0f / sampler->samplesPerPixel();

    for (int32_t y = orgy; y < maxy; ++y) {
        uint8_t *idp = PixelPtr(image, orgx, y);
        uint8_t *ndp = PixelPtr(normals, orgx, y);
        uint8_t *adp = PixelPtr(albedo, orgx, y);

        for (int32_t x = orgx; x < maxx; ++x) {
            Spectrum c;
            Spectrum A;
            v3f N;
            sampler->startPixel({ x, y });
            do {
                CameraSample cs = sampler->cameraSample();
                Ray r = ctx->m_camera->generateRay(cs);
                c += ctx->m_integrator->Li(r, ctx->m_scene, sampler, 0, &N, &A);
            } while (sampler->startNextSample());

            N *= ns_inv;
            v3f a = ApproxGammaCorrection((A * ns_inv).rgb());
            v3f Li = ApproxGammaCorrection((c * ns_inv).rgb());

            memcpy(idp, &Li.x, image.format.size);
            memcpy(ndp, &N.x, normals.format.size);
            memcpy(adp, &a.x, albedo.format.size);

            idp += image.format.size;
            ndp += normals.format.size;
            adp += albedo.format.size;
        }
    }
    ctx->m_event->signal();
}

static void Progress(const sptr<Object> &obj, const sptr<Object> &)
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
