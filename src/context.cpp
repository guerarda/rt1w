#include "context.hpp"

#include <vector>

#include "camera.hpp"
#include "event.hpp"
#include "integrator.hpp"
#include "sampler.hpp"
#include "sync.h"
#include "workq.hpp"

struct ImageTile : Object {
    static sptr<ImageTile> create(rect_t r, uint8_t *dp, size_t bpr) {
        return std::make_shared<ImageTile>(r, dp, bpr);
    }
    ImageTile(rect_t r, uint8_t *dp, size_t bpr) : m_rect(r), m_dp(dp), m_bytes_per_row(bpr) { }

    rect_t   m_rect;
    uint8_t *m_dp;
    size_t   m_bytes_per_row;
};

struct _RenderingContext : RenderingContext, std::enable_shared_from_this<_RenderingContext> {
    _RenderingContext(const sptr<Primitive> &world,
                      const sptr<Camera> &camera,
                      const sptr<Integrator> &integrator) : m_world(world),
                                                            m_camera(camera),
                                                            m_integrator(integrator) { }
    sptr<Event> schedule() override;
    buffer_t    buffer() override;

    sptr<Primitive>  m_world;
    sptr<Camera>     m_camera;
    sptr<Integrator> m_integrator;
    sptr<Event>      m_event;
    buffer_t         m_buffer;
    size_t           m_ntiles;
};

static void progress(const sptr<Object> &, const sptr<Object> &);
static void render_tile(const sptr<Object> &, const sptr<Object> &);

sptr<Event> _RenderingContext::schedule()
{
    v2u size = m_camera->resolution();
    m_buffer.format = buffer_format_init(TYPE_UINT8, ORDER_RGB);
    m_buffer.rect = { { 0, 0 }, size };

    m_buffer.bpr = size.x * m_buffer.format.size;
    m_buffer.data = malloc(size.y * m_buffer.bpr);

    /* Divide in tiles 32x32 */
    int32_t ntx = size.x / 32 + 1;
    int32_t nty = size.y / 32 + 1;

    std::vector<sptr<ImageTile>> tiles;
    for (int32_t i = 0; i < ntx; i++) {
        for (int32_t j = 0; j < nty; j++) {

            uint8_t *ptr = (uint8_t *)m_buffer.data + (uint32_t)j * 32 * m_buffer.bpr + (uint32_t)i * 32 * m_buffer.format.size;
            rect_t r;
            r.org.x = i * 32;
            r.org.y = j * 32;
            r.size.x = i < ntx - 1 ? 32 : 32 - ((uint32_t)ntx * 32 - size.x);
            r.size.y = j < nty - 1 ? 32 : 32 - ((uint32_t)nty * 32 - size.y);

            sptr<ImageTile> tile = ImageTile::create(r, ptr, m_buffer.bpr);
            tiles.push_back(tile);
        }
    }

    m_event = Event::create((int32_t )tiles.size());
    m_ntiles = tiles.size();

    for (const auto &t : tiles) {
        sptr<RenderingContext> ctx = shared_from_this();
        sptr<Event> e = workq_execute(workq_get_queue(), render_tile, ctx, t);
        e->notify(nullptr, progress, ctx, std::shared_ptr<Object>());
    }
    return m_event;
}

buffer_t _RenderingContext::buffer()
{
    schedule()->wait();
    return m_buffer;
}

#pragma mark - Static constructor

sptr<RenderingContext> RenderingContext::create(const sptr<Primitive> &world,
                                                const sptr<Camera> &camera,
                                                const sptr<Integrator> &integrator)
{
    return std::make_shared<_RenderingContext>(world, camera, integrator);
}

#pragma mark - Static functions

static void render_tile(const sptr<Object> &obj, const sptr<Object> &arg)
{
    sptr<_RenderingContext> ctx = std::static_pointer_cast<_RenderingContext>(obj);
    sptr<ImageTile> tile = std::static_pointer_cast<ImageTile>(arg);

    size_t nx = tile->m_rect.size.x;
    size_t ny = tile->m_rect.size.y;
    int32_t orgx = tile->m_rect.org.x;
    int32_t orgy = tile->m_rect.org.y;

    sptr<Sampler> sampler = ctx->m_integrator->sampler()->clone();

    float ns_inv = 1.0f / sampler->samplesPerPixel();

    for (size_t y = 0; y < ny; y++) {
        uint8_t *dp = tile->m_dp + (size_t)y * tile->m_bytes_per_row;

        for (size_t x = 0; x < nx; x++) {
            v3f c = { 0.0f, 0.0f, 0.0f };
            sampler->startPixel({ orgx + (int32_t)x, orgy + (int32_t)y });
            do {
                CameraSample cs = sampler->cameraSample();
                sptr<Ray> r = ctx->m_camera->generateRay(cs);

                c = c + ctx->m_integrator->Li(r, ctx->m_world, 0);
            } while (sampler->startNextSample());

            c *= ns_inv;

            /* Approx Gamma correction */
            c = {
                 fminf(1.0f, sqrtf(c.x)),
                 fminf(1.0f, sqrtf(c.y)),
                 fminf(1.0f, sqrtf(c.z))
            };

            dp[0] = (uint8_t)(255.99 * c.x);
            dp[1] = (uint8_t)(255.99 * c.y);
            dp[2] = (uint8_t)(255.99 * c.z);
            dp += 3;
        }
    }
    ctx->m_event->signal();
}

static void progress(const sptr<Object> &obj, const sptr<Object> &)
{
    static int32_t volatile progress = -1;

    int32_t done = sync_add_i32(&progress, 1);
    sptr<_RenderingContext> ctx = std::static_pointer_cast<_RenderingContext>(obj);

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
